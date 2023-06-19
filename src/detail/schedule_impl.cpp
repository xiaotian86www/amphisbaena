#include "schedule_impl.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace translator {

bool
timespec_gt(const timespec& left, const timespec& right)
{
  return left.tv_sec > right.tv_sec ||
         (left.tv_sec == right.tv_sec && left.tv_nsec > right.tv_nsec);
}

bool
CoTimerPtrGreater::operator()(const CoTimerPtr& left,
                              const CoTimerPtr& right) const
{
  return timespec_gt(left->timeout, right->timeout);
}

void
load_context(CoContext& main, const CoContext& in)
{
  memcpy(main.stack_.data() + main.stack_.size() - in.stack_.size(),
         in.stack_.data(),
         in.stack_.size());

  if (swapcontext(&main.uct_, &in.uct_) < 0) {
    perror("load_context");
    throw new std::runtime_error("load_context failed");
  }
}

void
store_context(const CoContext& main, CoContext& out)
{
  char dummy = 0;
  assert(main.stack_.data() <= &dummy); // 栈未被消耗完
  out.stack_.resize(main.stack_.data() + main.stack_.size() - &dummy);
  memcpy(out.stack_.data(), &dummy, out.stack_.size());

  if (swapcontext(&out.uct_, &main.uct_) < 0) {
    perror("store_context");
    throw new std::runtime_error("store_context failed");
  }
}

void
make_context(CoContext& main, CoContext& context, Schedule::Impl* impl)
{
  if (getcontext(&context.uct_) ==
      -1) { // 只是为了获取帧结构，以下动作完善帧结构
    perror("getcontext");
    throw new std::runtime_error("getcontext failed");
  }
  context.uct_.uc_stack.ss_sp = main.stack_.data();
  context.uct_.uc_stack.ss_size = main.stack_.size();
  context.uct_.uc_link = &main.uct_;
  // mainfunc只支持int类型参数，不支持void*参数属于设计问题
  uintptr_t ptr = (uintptr_t)impl;
  makecontext(&context.uct_,
              (void (*)())Schedule::Impl::co_func_wrapper,
              2,
              (uint32_t)ptr,
              (uint32_t)(ptr >> 32));

  // makecontext 后必须 swapcontext，否则会在协程执行结束后会出现异常退出的问题
  if (swapcontext(&main.uct_, &context.uct_) < 0) {
    perror("make_context");
    throw new std::runtime_error("make_context failed");
  }
}

Schedule::Impl::Impl(Schedule* sch, std::string_view socket_path)
  : co_count_(0)
  , socket_path_(socket_path)
{
  context_.stack_.resize(STACK_SIZE);

  epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
  if (event_fd_ < 0) {
    perror("eventfd");
    throw new std::runtime_error("eventfd failed");
  }

  event_fd_ = eventfd(0, EFD_NONBLOCK | EFD_SEMAPHORE);
  if (event_fd_ < 0) {
    perror("eventfd");
    throw new std::runtime_error("eventfd failed");
  }

  listen_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
  if (listen_fd_ < 0) {
    perror("socket");
    throw new std::runtime_error("socket failed");
  }
}

Schedule::Impl::~Impl()
{
  stop();

  // TODO 构造函数失败时，fd泄漏
  close(listen_fd_);
  close(event_fd_);
  close(epoll_fd_);
}

void
Schedule::Impl::run()
{
  epoll_event events[EPOLL_MAX_EVENTS];

  // 注册event_fd
  epoll_event event;
  event.data.fd = event_fd_;
  event.events = EPOLLIN;
  epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event_fd_, &event);

  while (co_count_ > 0) {
    int count = run_once();
    int timeout = run_timer();

    if (count > 0)
      timeout = 0;

    int len = epoll_wait(epoll_fd_, events, EPOLL_MAX_EVENTS, timeout);
    for (int i = 0; i < len; ++i) {
      if (events[i].data.fd == event_fd_) {
        assert(events[i].events == EPOLLIN);
        eventfd_t count;
        eventfd_read(event_fd_, &count);
      }
    }
  }
}

void
Schedule::Impl::stop()
{
  std::lock_guard<std::mutex> lg(mtx_);
  // 清除定时器
  while (!timers_que_.empty()) {
    timers_que_.pop();
  }

  // 清除待运行队列
  while (!running_cos_.empty()) {
    running_cos_.pop();
  }

  // 清除协程
  for (auto item : cos_) {
    switch (item->context.state_) {
      case CoroutineState::READY:
      case CoroutineState::SUSPEND:
        item->context.state_ = CoroutineState::DEAD;
        break;
      case CoroutineState::RUNNING:
        item->context.state_ = CoroutineState::DYING;
        break;
      case CoroutineState::DEAD:
        break;
      default:
        assert(false);
    }
  }
  co_count_ -= cos_.size();
  cos_.clear();

  eventfd_write(event_fd_, 1);
}

void
Schedule::Impl::post(task&& func)
{
  auto co = std::make_shared<Coroutine>();
  co->func = std::move(func);

  std::lock_guard<std::mutex> lg(mtx_);

  cos_.insert(co);
  co_count_++;

  running_cos_.push(co);

  eventfd_write(event_fd_, 1);
}

void
Schedule::Impl::yield()
{
  assert(running_);
  std::unique_lock<std::mutex> ul(mtx_);
  switch (running_->context.state_) {
    case CoroutineState::RUNNING:
      running_->context.state_ = CoroutineState::SUSPEND;
      break;
    case CoroutineState::DYING:
      running_->context.state_ = CoroutineState::DEAD;
      break;
    default:
      assert(false);
      break;
  }
  ul.unlock();

  store_context(context_, running_->context);
}

void
Schedule::Impl::yield_for(int milli)
{
  assert(running_);
  std::unique_lock<std::mutex> ul(mtx_);
  switch (running_->context.state_) {
    case CoroutineState::RUNNING: {
      auto timer = std::make_shared<CoTimer>();
      // 取系统启动时间，避免时间回调
      clock_gettime(CLOCK_MONOTONIC, &timer->timeout);
      timer->timeout.tv_nsec += milli * 1000000;
      timer->timeout.tv_sec += timer->timeout.tv_nsec / 1000000000;
      timer->co = running_;

      assert(!running_->timer);
      running_->timer = timer;

      timers_que_.push(timer);

      running_->context.state_ = CoroutineState::SUSPEND;
      break;
    }
    case CoroutineState::DYING:
      running_->context.state_ = CoroutineState::DEAD;
      break;
    default:
      assert(false);
      break;
  }
  ul.unlock();

  store_context(context_, running_->context);
}

void
Schedule::Impl::resume(CoroutinePtr co)
{
  std::unique_lock<std::mutex> ul(mtx_);

  running_cos_.push(co);

  eventfd_write(event_fd_, 1);
}

void
Schedule::Impl::co_func_wrapper(uint32_t low32, uint32_t high32)
{
  uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)high32 << 32);
  Schedule::Impl* impl = (Schedule::Impl*)ptr;
  assert(impl);
  impl->co_func();
}

void
Schedule::Impl::co_func()
{
  assert(running_);
  running_->func(ScheduleRef(shared_from_this()));

  std::lock_guard<std::mutex> lg(mtx_);
  switch (running_->context.state_) {
    case CoroutineState::RUNNING:
    case CoroutineState::DYING:
      running_->context.state_ = CoroutineState::DEAD;
      break;
    default:
      assert(false);
      break;
  }
  co_count_ -= cos_.erase(running_);
}

int
Schedule::Impl::run_timer()
{
  std::unique_lock<std::mutex> ul(mtx_);

  timespec now;
  // 取系统启动时间，避免时间回调
  clock_gettime(CLOCK_MONOTONIC, &now);

  while (!timers_que_.empty()) {
    auto timer = timers_que_.top();

    // 未触发
    if (timespec_gt(timer->timeout, now)) {
      // 向上取整
      return (timer->timeout.tv_sec - now.tv_sec) * 1000 +
                   (timer->timeout.tv_nsec - now.tv_nsec) / 1000000 +
                   (timer->timeout.tv_nsec - now.tv_nsec) % 1000000 >
                 0
               ? 1
               : 0;
    }

    timers_que_.pop();

    // 未取消
    if (auto sco = timer->co.lock();
        sco && sco->context.state_ != CoroutineState::DEAD &&
        sco->timer == timer) {
      sco->timer.reset();
      assert(sco->context.state_ == CoroutineState::SUSPEND);
      sco->context.state_ = CoroutineState::RUNNING;
      ul.unlock();

      do_resume(sco);

      return 0;
    }
  }

  return -1;
}

int
Schedule::Impl::run_once()
{
  std::unique_lock<std::mutex> ul(mtx_);

  while (!running_cos_.empty()) {
    auto co = running_cos_.front();
    running_cos_.pop();

    if (auto sco = co.lock();
        sco && sco->context.state_ != CoroutineState::DEAD) {
      sco->timer.reset();

      switch (sco->context.state_) {
        case CoroutineState::READY:
          sco->context.state_ = CoroutineState::RUNNING;
          ul.unlock();

          do_create(sco);
          break;
        case CoroutineState::SUSPEND:
          sco->context.state_ = CoroutineState::RUNNING;
          ul.unlock();

          do_resume(sco);
          break;
        default:
          assert(false);
      }

      return 1;
    }
  }

  return 0;
}

void
Schedule::Impl::do_listen()
{
  sockaddr_un serun;
  memset(&serun, 0, sizeof(serun));
  serun.sun_family = AF_UNIX;
  strcpy(serun.sun_path, socket_path_.c_str());
  int size = offsetof(sockaddr_un, sun_path) + strlen(serun.sun_path);
  // 删除sock文件
  unlink(socket_path_.c_str());
  if (bind(listen_fd_, (sockaddr*)&serun, size) < 0) {
    perror("bind");
    throw new std::runtime_error("listen failed");
  }

  if (listen(listen_fd_, 20) < 0) {
    perror("listen");
    throw new std::runtime_error("listen failed");
  }

  epoll_event event;
  event.data.fd = listen_fd_;
  event.events = EPOLLIN;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event_fd_, &event) < 0) {
    perror("epoll_ctl: listen");
    throw new std::runtime_error("listen failed");
  }
}

void
Schedule::Impl::do_create(std::shared_ptr<Coroutine> co)
{
  assert(!running_);
  running_ = co;
  make_context(context_, running_->context, this);
  running_ = nullptr;
}

void
Schedule::Impl::do_resume(std::shared_ptr<Coroutine> co)
{
  assert(!running_);
  running_ = co;
  load_context(context_, running_->context);
  running_ = nullptr;
}

}
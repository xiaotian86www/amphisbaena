#include "schedule_impl.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace translator {

void
load_context(CoContext& main, const CoContext& in)
{
  memcpy(main.stack_.data() + main.stack_.size() - in.stack_.size(),
         in.stack_.data(),
         in.stack_.size());

  swapcontext(&main.uct_, &in.uct_);
}

void
store_context(const CoContext& main, CoContext& out)
{
  char dummy = 0;
  assert(main.stack_.data() <= &dummy); // 栈未被消耗完
  out.stack_.resize(main.stack_.data() + main.stack_.size() - &dummy);
  memcpy(out.stack_.data(), &dummy, out.stack_.size());

  swapcontext(&out.uct_, &main.uct_);
}

void
make_context(CoContext& main, CoContext& context, Schedule::Impl* impl)
{
  getcontext(&context.uct_); // 只是为了获取帧结构，以下动作完善帧结构
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
  swapcontext(&main.uct_, &context.uct_);
}

Schedule::Impl::Impl(Schedule* sch)
  : co_count_(0)
{
  context_.stack_.resize(STACK_SIZE);

  event_fd = eventfd(0, EFD_NONBLOCK | EFD_SEMAPHORE);
  epoll_fd = epoll_create1(EPOLL_CLOEXEC);

  epoll_event event;
  event.data.fd = event_fd;
  event.events = EPOLLIN;

  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event_fd, &event);
}

Schedule::Impl::~Impl()
{
  stop();

  // TODO 构造函数失败时，fd泄漏
  close(epoll_fd);
  close(event_fd);
}

void
Schedule::Impl::run()
{
  epoll_event events[EPOLL_MAX_EVENTS];
  while (true) {
    int count = run_once();
    int timeout = run_timer();
    if (count == 0 && timeout != 0) {
      int len = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, timeout);
      for (int i = 0; i < len; ++i) {
        if (events[i].events == EPOLLIN) {
          eventfd_t count;
          eventfd_read(event_fd, &count);
        }
      }
    }

    if (co_count_ == 0) {
      break;
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

  eventfd_write(event_fd, 1);
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

  eventfd_write(event_fd, 1);
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

  // 存在已经调用stop方法，cos_为空，此时不应在继续投递任务
  if (std::lock_guard<std::mutex> lg(mtx_); cos_.find(running_) != cos_.end()) {

    auto timer = std::make_shared<CoTimer>();
    // 取系统启动时间，避免时间回调，自己写转换函数，实现四舍五入
    timer->timeout =
      (std::chrono::steady_clock::now().time_since_epoch().count() + 500000) /
        1000000 +
      milli;
    timer->co = running_;

    assert(!running_->timer);
    running_->timer = timer;

    timers_que_.push(timer);
  }

  yield();
}

void
Schedule::Impl::resume(CoroutinePtr co)
{
  std::unique_lock<std::mutex> ul(mtx_);

  running_cos_.push(co);

  eventfd_write(event_fd, 1);
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

  int now =
    (std::chrono::steady_clock::now().time_since_epoch().count() + 500000) /
    1000000;

  while (!timers_que_.empty()) {
    auto timer = timers_que_.top();

    // 未触发
    if (now < timer->timeout) {
      return timer->timeout - now;
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
#include "schedule_impl.hpp"

#include <chrono>
#include <mutex>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace translator {

void
load_context(CoContext& main, const CoContext& in)
{
  memcpy(
    &main.stack_.back() - in.stack_.size(), in.stack_.data(), in.stack_.size());

  swapcontext(&main.uct_, &in.uct_);
}

void
store_context(const CoContext& main, CoContext& out)
{
  char dummy = 0;
  assert(main.stack_.data() <= &dummy); // 栈未被消耗完
  out.stack_.resize(&main.stack_.back() - &dummy);
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
}

Schedule::Impl::Impl(Schedule* sch)
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
  while (true) {
    std::unique_lock<std::mutex> ul(mtx_);
    check_timer(ul, [this] { return !co_count_ || !running_cos_.empty(); });

    if (!co_count_)
      break;

    while (!running_cos_.empty() && !run_once(ul)) {
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
  co_count_ -= cos_.size();
  cos_.clear();

  eventfd_write(event_fd, 1);

  //   cv_.notify_all();
}

void
Schedule::Impl::post(task&& func)
{
  auto co = std::make_shared<Coroutine>();
  co->func = std::move(func);
  make_context(context_, co->context, this);

  std::lock_guard<std::mutex> lg(mtx_);

  cos_.insert(co);
  co_count_++;

  running_cos_.push(co);

  eventfd_write(event_fd, 1);
  //   cv_.notify_all();
}

void
Schedule::Impl::yield()
{
  assert(running_);
  auto co = running_;
  running_ = nullptr;
  store_context(context_, co->context);
}

void
Schedule::Impl::yield_for(const timespec& duration)
{
  assert(running_);

  // 存在已经调用stop方法，cos_为空，此时不应在继续投递任务
  if (std::lock_guard<std::mutex> lg(mtx_); cos_.find(running_) != cos_.end()) {

    auto timer = std::make_shared<CoTimer>();
    // 取系统启动时间，避免时间回调
    clock_gettime(CLOCK_MONOTONIC, &timer->timeout);
    timer->timeout.tv_nsec += duration.tv_nsec;
    timer->timeout.tv_sec += duration.tv_sec;
    // 调整进位
    timer->timeout.tv_sec += timer->timeout.tv_nsec / 1000000000;
    timer->timeout.tv_nsec = timer->timeout.tv_nsec % 1000000000;
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
  //   cv_.notify_all();
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
  auto co = running_;
  running_ = nullptr;

  std::lock_guard<std::mutex> lg(mtx_);
  co_count_ -= cos_.erase(co);
}

template<typename Func_>
void
Schedule::Impl::check_timer(std::unique_lock<std::mutex>& ul, Func_&& pred)
{
  epoll_event events[EPOLL_MAX_EVENTS];

  while (!timers_que_.empty()) {
    timespec n;
    clock_gettime(CLOCK_MONOTONIC, &n);

    auto timer = timers_que_.top();
    // 未触发
    if (n.tv_sec < timer->timeout.tv_sec ||
        (n.tv_sec == timer->timeout.tv_sec &&
         n.tv_nsec < timer->timeout.tv_nsec)) {
      std::chrono::milliseconds wait_time(
        (timer->timeout.tv_sec - n.tv_sec) * 1000 +
        (timer->timeout.tv_nsec - n.tv_nsec + 500000) /
          1000000); // 加500000为了四舍五入
      //   cv_.wait_for(ul, wait_time, std::forward<Func_>(pred));

      if (pred())
        return;

      ul.unlock();
      int len =
        epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, wait_time.count());
      for (int i = 0; i < len; ++i) {
        if (events[i].events == EPOLLIN) {
          eventfd_t count;
          eventfd_read(event_fd, &count);
        }
      }

      ul.lock(); // TODO 抛出异常时该行没有执行

      return;
    }

    timers_que_.pop();

    // 未取消
    auto sco = timer->co.lock();
    if (sco && cos_.find(sco) != cos_.end() && sco->timer == timer) {
      assert(!running_);
      assert(sco->timer);
      sco->timer.reset();

      running_ = sco;
      ul.unlock();
      load_context(context_, running_->context);
      ul.lock();
      return;
    }
  }

  if (pred())
    return;

  ul.unlock();
  int len = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, -1);
  for (int i = 0; i < len; ++i) {
    if (events[i].events == EPOLLIN) {
      eventfd_t count;
      eventfd_read(event_fd, &count);
    }
  }

  ul.lock(); // TODO 抛出异常时该行没有执行
}

bool
Schedule::Impl::run_once(std::unique_lock<std::mutex>& ul)
{
  assert(!running_cos_.empty());

  auto co = running_cos_.front();
  running_cos_.pop();

  auto sco = co.lock();
  if (sco && cos_.find(sco) != cos_.end()) {
    sco->timer.reset();

    assert(!running_);
    running_ = sco;
    ul.unlock();
    load_context(context_, running_->context);
    ul.lock();

    return true;
  }

  return false;
}

}
#include "schedule.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <memory>
#include <mutex>
#include <queue>
#include <utility>

#define STACK_SIZE (1024 * 1024)

namespace translator {

struct CoContext
{
  std::vector<char> stack_;
  ucontext_t uct_;
};

struct CoTimer
{
  timespec timeout;
  Schedule::CoroutinePtr co;
};

typedef std::shared_ptr<CoTimer> CoTimerPtr;

struct CoTimerPtrGreater
{
  bool operator()(const CoTimerPtr& left, const CoTimerPtr& right) const
  {
    return left->timeout.tv_sec > right->timeout.tv_sec ||
           (left->timeout.tv_sec == right->timeout.tv_sec &&
            left->timeout.tv_nsec > right->timeout.tv_nsec);
  }
};

struct Schedule::Coroutine
{
  Coroutine() = default;

  task func;
  CoContext context;
  CoTimerPtr timer;
};

static void
co_func_wrapper(uint32_t low32, uint32_t high32);

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
              (void (*)())co_func_wrapper,
              2,
              (uint32_t)ptr,
              (uint32_t)(ptr >> 32));
}

class Schedule::Impl : public std::enable_shared_from_this<Schedule::Impl>
{
public:
  Impl(Schedule* sch) { context_.stack_.resize(STACK_SIZE); }

  ~Impl() { stop(); }

public:
  void run()
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

  void stop()
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

    cv_.notify_all();
  }

  void post(task&& func)
  {
    auto co = std::make_shared<Coroutine>();
    co->func = std::move(func);
    make_context(context_, co->context, this);

    std::lock_guard<std::mutex> lg(mtx_);

    cos_.insert(co);
    co_count_++;

    running_cos_.push(co);
    cv_.notify_all();
  }

  void yield()
  {
    assert(running_);
    auto co = running_;
    running_ = nullptr;
    store_context(context_, co->context);
  }

  void yield_for(const timespec& duration)
  {
    assert(running_);

    // 存在已经调用stop方法，cos_为空，此时不应在继续投递任务
    if (std::lock_guard<std::mutex> lg(mtx_);
        cos_.find(running_) != cos_.end()) {

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

  void resume(CoroutinePtr co)
  {
    std::unique_lock<std::mutex> ul(mtx_);

    running_cos_.push(co);
    cv_.notify_all();
  }

  CoroutinePtr this_co()
  {
    assert(running_);
    return running_;
  }

  void increase()
  {
    std::lock_guard<std::mutex> lg(mtx_);
    ++co_count_;
    cv_.notify_all();
  }

  void decrease()
  {
    std::lock_guard<std::mutex> lg(mtx_);
    --co_count_;
    cv_.notify_all();
  }

  void co_func()
  {
    assert(running_);
    running_->func(ScheduleRef(shared_from_this()));
    auto co = running_;
    running_ = nullptr;

    std::lock_guard<std::mutex> lg(mtx_);
    co_count_ -= cos_.erase(co);
  }

private:
  template<typename Func_>
  void check_timer(std::unique_lock<std::mutex>& ul, Func_&& pred)
  {
    timespec n;
    clock_gettime(CLOCK_MONOTONIC, &n);
    while (!timers_que_.empty()) {
      auto timer = timers_que_.top();
      // 未触发
      if (n.tv_sec < timer->timeout.tv_sec ||
          (n.tv_sec == timer->timeout.tv_sec &&
           n.tv_nsec < timer->timeout.tv_nsec)) {
        std::chrono::nanoseconds wait_time(
          (n.tv_sec - timer->timeout.tv_sec) * 1000 * 1000 * 1000 +
          (n.tv_nsec - timer->timeout.tv_nsec));
        cv_.wait_for(ul, wait_time, std::forward<Func_>(pred));
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

    cv_.wait(ul, std::forward<Func_>(pred));
  }

  bool run_once(std::unique_lock<std::mutex>& ul)
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

private:
  CoContext context_;
  std::unordered_set<std::shared_ptr<Coroutine>> cos_;
  std::shared_ptr<Coroutine> running_;
  std::queue<CoroutinePtr> running_cos_;
  int32_t co_count_ = 0;
  std::priority_queue<CoTimerPtr, std::vector<CoTimerPtr>, CoTimerPtrGreater>
    timers_que_;
  std::mutex mtx_;
  std::condition_variable cv_;
};

Schedule::Worker::Worker(Schedule& sch)
  : sch_(sch)
{
  sch_.impl_->increase();
}

Schedule::Worker::~Worker()
{
  sch_.impl_->decrease();
}

void
co_func_wrapper(uint32_t low32, uint32_t high32)
{
  uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)high32 << 32);
  Schedule::Impl* impl = (Schedule::Impl*)ptr;
  assert(impl);
  impl->co_func();
}

Schedule::Schedule()
  : impl_(std::make_shared<Impl>(this))
{
}

Schedule::~Schedule() {}

void
Schedule::run()
{
  impl_->run();
}

void
Schedule::stop()
{
  impl_->stop();
}

void
Schedule::post(task&& func)
{
  impl_->post(std::move(func));
}

void
Schedule::yield()
{
  impl_->yield();
}

void
Schedule::resume(CoroutinePtr co)
{
  impl_->resume(co);
}

void
Schedule::yield_for_(const timespec& rtime)
{
  impl_->yield_for(rtime);
}

Schedule::CoroutinePtr
Schedule::this_co()
{
  return impl_->this_co();
}

ScheduleRef::ScheduleRef(std::weak_ptr<Schedule::Impl> impl)
  : ptr_(impl)
{
}

void
ScheduleRef::stop()
{
  auto impl = ptr_.lock();
  if (impl)
    impl->stop();
}

void
ScheduleRef::post(task&& func)
{
  auto impl = ptr_.lock();
  if (impl)
    impl->post(std::move(func));
}

void
ScheduleRef::resume(Schedule::CoroutinePtr co)
{
  auto impl = ptr_.lock();
  if (impl)
    impl->resume(co);
}

void
ScheduleRef::yield()
{
  auto impl = ptr_.lock();
  if (impl)
    impl->yield();
}

void
ScheduleRef::yield_for_(const timespec& rtime)
{
  auto impl = ptr_.lock();
  if (impl)
    impl->yield_for(rtime);
}

Schedule::CoroutinePtr
ScheduleRef::this_co()
{
  auto impl = ptr_.lock();
  if (impl)
    return impl->this_co();
  else
    return Schedule::CoroutinePtr();
}

} // namespace translator

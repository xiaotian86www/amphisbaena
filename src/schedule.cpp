#include "schedule.hpp"

#include <bits/types/struct_timespec.h>
#include <bits/types/time_t.h>
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

struct Schedule::Coroutine
{
  Coroutine() = default;

  CoContext context;
  task func;
};

struct CoTimer
{
  timespec timeout;
  std::shared_ptr<Schedule::Coroutine> co;
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

      if (!running_cos_.empty())
        run_once(ul);
    }
  }

  void stop()
  {
    std::lock_guard<std::mutex> lg(mtx_);
    // 清除定时器
    while (!timers_que_.empty()) {
      timers_que_.pop();
    }

    timers_.clear();

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
      timer->co = { running_ };

      assert(timers_.find(running_) == timers_.end());
      timers_.insert(std::make_pair(running_, timer));
      timers_que_.push(timer);
    }

    yield();
  }

  void resume(CoroutineRef co)
  {
    auto sco = co.ptr_.lock();
    if (!sco)
      return;

    std::unique_lock<std::mutex> ul(mtx_);
    // 存在已经调用stop方法，cos_为空，此时不应在继续投递任务
    if (cos_.find(sco) == cos_.end())
      return;

    // 取消定时器
    timers_.erase(sco);

    running_cos_.push(sco);
    cv_.notify_all();
  }

  CoroutineRef this_co()
  {
    assert(running_);
    return CoroutineRef{ running_ };
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
      if (auto iter = timers_.find(timer->co);
          iter != timers_.end() && iter->second->co == timer->co) {
        assert(!running_);
        running_ = timer->co;
        timers_.erase(iter);
        ul.unlock();
        load_context(context_, running_->context);
        ul.lock();
        return;
      }
    }

    cv_.wait(ul, std::forward<Func_>(pred));
  }

  void run_once(std::unique_lock<std::mutex>& ul)
  {
    assert(!running_);
    assert(!running_cos_.empty());

    running_ = running_cos_.front();
    running_cos_.pop();

    ul.unlock();
    load_context(context_, running_->context);
    ul.lock();
  }

private:
  CoContext context_;
  std::unordered_set<std::shared_ptr<Coroutine>> cos_;
  std::shared_ptr<Coroutine> running_;
  std::queue<std::shared_ptr<Coroutine>> running_cos_;
  int32_t co_count_;
  std::priority_queue<CoTimerPtr, std::vector<CoTimerPtr>, CoTimerPtrGreater>
    timers_que_;
  std::unordered_map<std::shared_ptr<Coroutine>, CoTimerPtr>
    timers_; // TODO 删除时，以Coroutinue为查找条件
  std::mutex mtx_;
  std::condition_variable cv_;
};

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
Schedule::resume(CoroutineRef co)
{
  impl_->resume(co);
}

void
Schedule::yield_for_(const timespec& rtime)
{
  impl_->yield_for(rtime);
}

Schedule::CoroutineRef
Schedule::this_co()
{
  return impl_->this_co();
}

ScheduleRef::ScheduleRef(std::weak_ptr<Schedule::Impl> impl)
  : ptr_(impl)
{
}

void
ScheduleRef::post(task&& func)
{
  auto impl = ptr_.lock();
  if (impl)
    impl->post(std::move(func));
}

void
ScheduleRef::resume(Schedule::CoroutineRef co)
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

Schedule::CoroutineRef
ScheduleRef::this_co()
{
  auto impl = ptr_.lock();
  if (impl)
    return impl->this_co();
  else
    return { std::weak_ptr<Schedule::Coroutine>() };
}

} // namespace translator

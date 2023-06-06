#include "schedule.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <queue>

#define STACK_SIZE (1024 * 1024)

namespace translator {

struct CoContext
{
  std::vector<char> stack_;
  ucontext_t uct_;
};

struct Schedule::Coroutine : public std::enable_shared_from_this<Coroutine>
{
  Coroutine() = default;

  CoContext context;
  task func;
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
      std::shared_ptr<Coroutine> co;
      {
        std::unique_lock<std::mutex> ul(mtx_);
        cv_.wait(ul, [this] { return !co_count_ || !running_cos_.empty(); });

        if (!co_count_)
          break;

        co = running_cos_.front();
        running_cos_.pop();
      }

      assert(!running_);
      running_ = co;

      load_context(context_, co->context);
    }
  }

  void stop()
  {
    std::lock_guard<std::mutex> lg(mtx_);
    while (!running_cos_.empty()) {
      auto co = running_cos_.front();
      running_cos_.pop();

      co_count_--;
      cos_.erase(co);
    }

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
    // resume(co);
    store_context(context_, co->context);
  }

  void resume(CoroutineRef co)
  {
    auto sco = co.ptr_.lock();
    if (!sco)
      return;

    std::unique_lock<std::mutex> ul(mtx_);
    if (cos_.find(sco) == cos_.end()) // 不属于该sch的co
      return;

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
    co_count_--;
    cos_.erase(co);
  }

private:
  CoContext context_;
  std::unordered_set<std::shared_ptr<Coroutine>> cos_;
  std::shared_ptr<Coroutine> running_;
  std::queue<std::shared_ptr<Coroutine>> running_cos_;
  int32_t co_count_;
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

#include "schedule.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <queue>

#define STACK_SIZE (1024 * 1024)

namespace translator {

// thread_local std::shared_ptr<Schedule> sch;

void
load_context(Schedule::Context& main, const Schedule::Context& in)
{
  memcpy(
    &main.stack_.back() - in.stack_.size(), in.stack_.data(), in.stack_.size());

  swapcontext(&main.uct_, &in.uct_);
}

void
store_context(const Schedule::Context& main, Schedule::Context& out)
{
  char dummy = 0;
  assert(main.stack_.data() <= &dummy); // 栈未被消耗完
  out.stack_.resize(&main.stack_.back() - &dummy);
  memcpy(out.stack_.data(), &dummy, out.stack_.size());

  swapcontext(&out.uct_, &main.uct_);
}

void
make_context(Schedule::Context& main,
             Schedule::Context& context,
             void (*func)(Schedule*),
             Schedule* sch)
{
  getcontext(&context.uct_); // 只是为了获取帧结构，以下动作完善帧结构
  context.uct_.uc_stack.ss_sp = main.stack_.data();
  context.uct_.uc_stack.ss_size = main.stack_.size();
  context.uct_.uc_link = &main.uct_;
  // mainfunc只支持int类型参数，不支持void*参数属于设计问题
  uintptr_t ptr = (uintptr_t)sch;
  makecontext(
    &context.uct_, (void (*)())func, 2, (uint32_t)ptr, (uint32_t)(ptr >> 32));
}

struct Schedule::Coroutine : public std::enable_shared_from_this<Coroutine>
{
  Coroutine() = default;

  Context context;
  task func;
};

Schedule::Schedule()
{
  context_.stack_.resize(STACK_SIZE);
}

Schedule::~Schedule()
{
  stop();
}

void
Schedule::run()
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

void
Schedule::stop()
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

void
Schedule::post(task&& func)
{
  auto co = std::make_shared<Coroutine>();
  co->func = std::move(func);
  make_context(context_, co->context, (void (*)(Schedule*))co_func, this);

  std::lock_guard<std::mutex> lg(mtx_);

  cos_.insert(co);
  co_count_++;

  running_cos_.push(co);
  cv_.notify_all();
}

void
Schedule::yield()
{
  assert(running_);
  auto co = running_;
  running_ = nullptr;
  // resume(co);
  store_context(context_, co->context);
}

void
Schedule::resume(std::weak_ptr<Coroutine> co)
{
  auto sco = co.lock();
  if (!sco)
    return;

  std::unique_lock<std::mutex> ul(mtx_);
  if (cos_.find(sco) == cos_.end()) // 不属于该sch的co
    return;

  running_cos_.push(sco);
  cv_.notify_all();
}

std::weak_ptr<Schedule::Coroutine>
Schedule::this_co()
{
  assert(running_);
  return running_;
}

void
Schedule::co_func(uint32_t low32, uint32_t high32)
{
  uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)high32 << 32);
  Schedule* sch = (Schedule*)ptr;
  assert(sch);
  assert(sch->running_);
  sch->running_->func(sch);
  auto co = sch->running_;
  sch->running_ = nullptr;

  std::lock_guard<std::mutex> lg(sch->mtx_);
  sch->co_count_--;
  sch->cos_.erase(co);
}

} // namespace translator

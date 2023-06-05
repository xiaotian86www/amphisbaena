#include "schedule.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <queue>

#define STACK_SIZE (1024 * 1024)

namespace translator {

thread_local std::shared_ptr<Schedule> sch;

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
             void (*func)(void))
{
  getcontext(&context.uct_); // 只是为了获取帧结构，以下动作完善帧结构
  context.uct_.uc_stack.ss_sp = main.stack_.data();
  context.uct_.uc_stack.ss_size = main.stack_.size();
  context.uct_.uc_link = &main.uct_;
  // mainfunc只支持int类型参数，不支持void*参数属于设计问题
  makecontext(&context.uct_, func, 0);
}

struct Schedule::Coroutine : public std::enable_shared_from_this<Coroutine>
{
  Coroutine() = default;

  Context context;
  task func;
  int id;
};

Schedule::Schedule()
{
  context_.stack_.resize(STACK_SIZE);
}

Schedule::~Schedule()
{
  // stop();
}

void
Schedule::run()
{
  assert(!sch);
  sch = shared_from_this();

  th_running_ = true;

  while (co_count_) {
    std::shared_ptr<Coroutine> co;
    {
      std::unique_lock<std::mutex> ul(running_cos_mtx_);
      running_cos_cv_.wait(
        ul, [this] { return !running_cos_.empty(); });

      co = running_cos_.front();
      running_cos_.pop();
    }

    assert(!running_);
    running_ = co;

    assert(sch);
    load_context(sch->context_, co->context);
  }

  sch.reset();
}

void
Schedule::stop()
{
  // th_running_ = false;
  // running_cos_cv_.notify_all();
}

void
Schedule::post(task&& func)
{
  auto co = co_create(std::move(func));
  resume(co);
}

void
Schedule::yield()
{
  assert(sch);
  assert(sch->running_);
  auto co = sch->running_;
  sch->running_ = nullptr;
  // sch->resume(co);
  store_context(sch->context_, co->context);
}

void
Schedule::resume(std::weak_ptr<Coroutine> co)
{
  auto sco = co.lock();
  if (!sco)
    return;

  std::unique_lock<std::mutex> ul(running_cos_mtx_);
  running_cos_.push(sco);
  running_cos_cv_.notify_all();
}

std::weak_ptr<Schedule::Coroutine>
Schedule::this_co()
{
  assert(sch);
  assert(sch->running_);
  return sch->running_;
}

std::shared_ptr<Schedule>
Schedule::this_sch()
{
  assert(sch);
  return sch;
}

void
Schedule::th_func()
{
  assert(!sch);
  sch = shared_from_this();

  while (co_count_) {
    std::shared_ptr<Coroutine> co;
    {
      std::unique_lock<std::mutex> ul(running_cos_mtx_);
      running_cos_cv_.wait(
        ul, [this] { return !running_cos_.empty(); });

      co = running_cos_.front();
      running_cos_.pop();
    }

    assert(!running_);
    running_ = co;

    assert(sch);
    load_context(sch->context_, co->context);
  }

  sch.reset();
}

void
Schedule::co_func()
{
  assert(sch);
  assert(sch->running_);
  sch->running_->func();
  auto co = sch->running_;
  sch->running_ = nullptr;

  sch->co_destroy(co);
}

std::weak_ptr<Schedule::Coroutine>
Schedule::co_create(task&& func)
{
  auto co = std::make_shared<Coroutine>();
  co->func = std::move(func);
  make_context(context_, co->context, (void (*)(void))co_func);

  std::lock_guard<std::mutex> lg(cos_mtx_);

  cos_.insert(co);

  co_count_++;

  return co;
}

void
Schedule::co_destroy(std::shared_ptr<Coroutine> co)
{
  std::lock_guard<std::mutex> lg(cos_mtx_);
  co_count_--;
  cos_.erase(co);
}

} // namespace translator

#include "coroutine.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <queue>

namespace translator {

thread_local Schedule* sch;

// void
// co_func(uint32_t low32, uint32_t hi32);

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

Schedule::Context
make_context(Schedule::Context& main, void (*func)(void))
{
  Schedule::Context context;
  getcontext(&context.uct_); // 只是为了获取帧结构，以下动作完善帧结构
  context.uct_.uc_stack.ss_sp = main.stack_.data();
  context.uct_.uc_stack.ss_size = main.stack_.size();
  context.uct_.uc_link = &main.uct_;
  // mainfunc只支持int类型参数，不支持void*参数属于设计问题
  makecontext(&context.uct_, func, 0);

  return context;
}

class Schedule::Coroutine
{
public:
  enum class StatusEnum : int
  {
    COROUTINE_DEAD = 0,
    COROUTINE_READY = 1,
    COROUTINE_RUNNING = 2,
    COROUTINE_SUSPEND = 3
  };

public:
  Coroutine(int id, Context&& context, task&& func)
    : context_(std::move(context))
    , func_(std::move(func))
    , id_(id)
  {
  }

public:
  void resume();

  void yield();

public:
  StatusEnum status() { return status_; }

  int id() { return id_; }

private:
  // static void co_func(uint32_t low32, uint32_t hi32);

public:
  Schedule::Context context_;
  task func_;
  StatusEnum status_ = StatusEnum::COROUTINE_READY;
  int id_;
};

// void
// co_func(uint32_t low32, uint32_t hi32)
// {
//   uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
//   Schedule* sch = (Schedule*)ptr;
//   sch->co_func();
// }

Schedule::Schedule()
  : th_(std::bind(&Schedule::th_func, this))
{
  sch = this;
  context_.stack_.resize(STACK_SIZE);
}

Schedule::~Schedule()
{
  th_running_ = false;
  running_cos_cv_.notify_all();
  if (th_.joinable())
    th_.join();

  sch = nullptr;
}

int
Schedule::create(task&& func)
{
  int id;
  {
    std::lock_guard<std::mutex> lg(cos_mtx_);

    if (free_ids_.empty()) {
      id = cos_.size();
      cos_.push_back(nullptr);
    } else {
      id = free_ids_.top();
      free_ids_.pop();
    }
    auto ptr = std::make_unique<Coroutine>(
      id, make_context(context_, (void (*)(void))co_func), std::move(func));
    cos_[id] = std::move(ptr);
  }
  resume(id);

  return id;
}

void
Schedule::destroy(int id)
{
  std::lock_guard<std::mutex> lg(cos_mtx_);
  if (cos_[id] == cos_.back()) {
    cos_.pop_back();
  } else {
    sch->cos_[id].release();
    sch->free_ids_.push(id);
  }
}

void
Schedule::yield()
{
  assert(running_);
  auto co = running_;
  running_ = nullptr;
  co->yield();
}

void
Schedule::resume(int id)
{
  assert(id < cos_.size());
  assert(cos_[id]);

  std::unique_lock<std::mutex> ul(running_cos_mtx_);
  running_cos_.push(cos_[id].get());
  running_cos_cv_.notify_all();
}

int
Schedule::this_co_id()
{
  return running_->id();
}

Schedule*
Schedule::this_sch()
{
  return sch;
}

void
Schedule::th_func()
{
  sch = this;

  while (true) {
    Coroutine* co;
    {
      std::unique_lock<std::mutex> ul(running_cos_mtx_);
      running_cos_cv_.wait(
        ul, [this] { return !th_running_ || !running_cos_.empty(); });

      if (!th_running_ && running_cos_.empty())
        break;

      co = running_cos_.front();
      running_cos_.pop();
    }

    assert(!running_);
    running_ = co;
    co->resume();
  }

  sch = nullptr;
}

void
Schedule::co_func()
{
  assert(sch);
  assert(sch->running_);
  sch->running_->func_();
  auto co = sch->running_;
  sch->running_ = nullptr;

  sch->destroy(co->id_);
}

void
Schedule::Coroutine::resume()
{
  switch (status_) {
    case StatusEnum::COROUTINE_READY:
    case StatusEnum::COROUTINE_SUSPEND: {
      status_ = StatusEnum::COROUTINE_RUNNING;
      assert(sch);
      load_context(sch->context_, context_);
      break;
    }
    default:
      assert(0);
  }
}

void
Schedule::Coroutine::yield()
{
  status_ = StatusEnum::COROUTINE_SUSPEND;
  assert(sch);
  store_context(sch->context_, context_);
}

// void
// Schedule::Coroutine::co_func(uint32_t low32, uint32_t hi32)
// {
//   uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
//   Coroutine* co = (Coroutine*)ptr;
//   co->func_();
//   sch->running_ = nullptr;
//   co->status_ = StatusEnum::COROUTINE_DEAD;
//   sch->destroy(co->id_);
// }

} // namespace translator

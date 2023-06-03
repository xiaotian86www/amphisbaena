#include "coroutine.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <queue>

namespace translator {

thread_local Schedule* sch;

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
  Coroutine(int id, task&& func);

  ~Coroutine();

public:
  void resume();

  void yield();

public:
  StatusEnum status() { return status_; }

  int id() { return id_; }

private:
  static void mainfunc(uint32_t low32, uint32_t hi32);

private:
  std::vector<char> stack_;
  task func_;
  ucontext_t ctx_;
  StatusEnum status_ = StatusEnum::COROUTINE_READY;
  int id_;
};

Schedule::Schedule()
  : th_(std::bind(&Schedule::th_func, this))
{
  sch = this;
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
    auto ptr = std::make_unique<Coroutine>(id, std::move(func));
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
  running_->yield();
}

void
Schedule::resume(int id)
{
  assert(id < cos_.size());
  assert(cos_[id]);

  std::unique_lock<std::mutex> ul(running_cos_mtx_);
  running_cos_.push(cos_[id].get());
}

int
Schedule::running_id()
{
  return running_->id();
}

Schedule*
Schedule::current_schedule()
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

      if (!running_)
        break;

      co = running_cos_.front();
      running_cos_.pop();
    }

    co->resume();
  }

  sch = nullptr;
}

Schedule::Coroutine::Coroutine(int id, task&& func)
  : func_(std::move(func))
  , id_(id)
{
}

Schedule::Coroutine::~Coroutine() {}

void
Schedule::Coroutine::resume()
{
  assert(!sch->running_);
  switch (status_) {
    case StatusEnum::COROUTINE_READY: {
      getcontext(&ctx_); // 只是为了获取帧结构，以下动作完善帧结构
      ctx_.uc_stack.ss_sp = sch->stack_;
      ctx_.uc_stack.ss_size = STACK_SIZE;
      ctx_.uc_link = &sch->main_;
      sch->running_ = this;
      status_ = StatusEnum::COROUTINE_RUNNING;
      uintptr_t ptr = (uintptr_t)this;
      // mainfunc只支持int类型参数，不支持void*参数属于设计问题
      makecontext(&ctx_,
                  (void (*)(void))mainfunc,
                  2,
                  (uint32_t)ptr,
                  (uint32_t)(ptr >> 32));
      swapcontext(&sch->main_, &ctx_); // 保存主线程帧，设置协程帧为当前帧
      break;
    }
    case StatusEnum::COROUTINE_SUSPEND: {
      memcpy(
        sch->stack_ + STACK_SIZE - stack_.size(), stack_.data(), stack_.size());
      sch->running_ = this;
      status_ = StatusEnum::COROUTINE_RUNNING;
      swapcontext(&sch->main_, &ctx_);
      break;
    }
    default:
      assert(0);
  }
}

void
Schedule::Coroutine::yield()
{
  assert(sch);
  assert(sch->running_);
  char dummy = 0;
  // 栈未被消耗完
  assert(sch->stack_ <= &dummy);
  stack_.resize(sch->stack_ + STACK_SIZE - &dummy);
  memcpy(stack_.data(), &dummy, stack_.size());
  status_ = StatusEnum::COROUTINE_SUSPEND;
  sch->running_ = nullptr;
  swapcontext(&ctx_, &sch->main_);
}

void
Schedule::Coroutine::mainfunc(uint32_t low32, uint32_t hi32)
{
  uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
  Coroutine* co = (Coroutine*)ptr;
  co->func_();
  co->status_ = StatusEnum::COROUTINE_DEAD;
  sch->running_ = nullptr;
  sch->destroy(co->id_);
}

void co_yield ()
{
  assert(sch);
  sch->yield();
}

void
co_resume(int id)
{
  assert(sch);
  sch->resume(id);
}

int
co_id()
{
  assert(sch);
  return sch->running_id();
}

} // namespace translator

#include "coroutine.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <queue>

namespace translator {

thread_local Schedule* sc;

void
CoroutineDeleter::operator()(Coroutine* co) const
{
  assert(sc);
  sc->destroy(co);
}

Schedule::Schedule()
{
  sc = this;
}

Schedule::~Schedule()
{
  sc = nullptr;
}

CoroutinePtr
Schedule::create(coroutine_func&& func)
{
  int id;
  if (free_ids_.empty()) {
    id = cos_.size();
    cos_.push_back(nullptr);
  } else {
    id = free_ids_.top();
    free_ids_.pop();
  }

  CoroutinePtr ptr(new Coroutine(id, std::move(func)));
  cos_[id] = ptr.get();
  return ptr;
}

void
Schedule::destroy(Coroutine* co)
{
  if (co == cos_.back()) {
    cos_.pop_back();
  } else {
    sc->cos_[co->id()] = nullptr;
    sc->free_ids_.push(co->id());
  }
}

Coroutine::Coroutine(int id, coroutine_func&& func)
  : func_(std::move(func))
  , id_(id)
{
}

Coroutine::~Coroutine()
{
}

void
Coroutine::resume()
{
  assert(!sc->running_);
  switch (status_) {
    case StatusEnum::COROUTINE_READY: {
      getcontext(&ctx_); // 只是为了获取帧结构，以下动作完善帧结构
      ctx_.uc_stack.ss_sp = sc->stack_;
      ctx_.uc_stack.ss_size = STACK_SIZE;
      ctx_.uc_link = &sc->main_;
      sc->running_ = this;
      status_ = StatusEnum::COROUTINE_RUNNING;
      uintptr_t ptr = (uintptr_t)this;
      // mainfunc只支持int类型参数，不支持void*参数属于设计问题
      makecontext(&ctx_,
                  (void (*)(void))mainfunc,
                  2,
                  (uint32_t)ptr,
                  (uint32_t)(ptr >> 32));
      swapcontext(&sc->main_, &ctx_); // 保存主线程帧，设置协程帧为当前帧
      break;
    }
    case StatusEnum::COROUTINE_SUSPEND: {
      memcpy(
        sc->stack_ + STACK_SIZE - stack_.size(), stack_.data(), stack_.size());
      sc->running_ = this;
      status_ = StatusEnum::COROUTINE_RUNNING;
      swapcontext(&sc->main_, &ctx_);
      break;
    }
    default:
      assert(0);
  }
}

void
Coroutine::yield()
{
  assert(sc->running_);
  char dummy = 0;
  // 栈未被消耗完
  assert(sc->stack_ <= &dummy);
  stack_.resize(sc->stack_ + STACK_SIZE - &dummy);
  memcpy(stack_.data(), &dummy, stack_.size());
  status_ = StatusEnum::COROUTINE_SUSPEND;
  sc->running_ = nullptr;
  swapcontext(&ctx_, &sc->main_);
}

void
Coroutine::mainfunc(uint32_t low32, uint32_t hi32)
{
  uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
  Coroutine* co = (Coroutine*)ptr;
  co->func_();
  co->status_ = StatusEnum::COROUTINE_DEAD;
  sc->running_ = nullptr;
}

void co_yield ()
{
  sc->running_->yield();
}

void
co_resume(int id)
{
  assert(id < sc->cos_.size());
  assert(sc->cos_[id]);
  sc->cos_[id]->resume();
}

int
co_id()
{
  assert(sc->running_);
  return sc->running_->id();
}
} // namespace translator

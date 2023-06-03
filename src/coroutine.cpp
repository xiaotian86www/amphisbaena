#include "coroutine.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <queue>

struct Schedule
{
  char stack[STACK_SIZE];
  ucontext_t main;
  Coroutine* running = nullptr;
  std::priority_queue<int, std::vector<int>, std::greater<int>> free_ids;
  std::vector<Coroutine*> cos;
};

thread_local Schedule sc;

Coroutine::Coroutine(Coroutine::coroutine_func&& func)
  : func_(std::move(func))
  , status_(StatusEnum::COROUTINE_READY)
{
  if (sc.free_ids.empty()) {
    id_ = sc.cos.size();
    sc.cos.push_back(this);
  } else {
    id_ = sc.free_ids.top();
    sc.free_ids.pop();
    sc.cos[id_] = this;
  }
}

Coroutine::~Coroutine()
{
  if (this == sc.cos.back()) {
    sc.cos.pop_back();
  } else {
    sc.cos[id_] = nullptr;
    sc.free_ids.push(id_);
  }
}

void
Coroutine::resume()
{
  assert(!sc.running);
  switch (status_) {
    case StatusEnum::COROUTINE_READY: {
      getcontext(&ctx_); // 只是为了获取帧结构，以下动作完善帧结构
      ctx_.uc_stack.ss_sp = sc.stack;
      ctx_.uc_stack.ss_size = STACK_SIZE;
      ctx_.uc_link = &sc.main;
      sc.running = this;
      status_ = StatusEnum::COROUTINE_RUNNING;
      uintptr_t ptr = (uintptr_t)this;
      // mainfunc只支持int类型参数，不支持void*参数属于设计问题
      makecontext(&ctx_,
                  (void (*)(void))mainfunc,
                  2,
                  (uint32_t)ptr,
                  (uint32_t)(ptr >> 32));
      swapcontext(&sc.main, &ctx_); // 保存主线程帧，设置协程帧为当前帧
      break;
    }
    case StatusEnum::COROUTINE_SUSPEND: {
      memcpy(
        sc.stack + STACK_SIZE - stack_.size(), stack_.data(), stack_.size());
      sc.running = this;
      status_ = StatusEnum::COROUTINE_RUNNING;
      swapcontext(&sc.main, &ctx_);
      break;
    }
    default:
      assert(0);
  }
}

void
Coroutine::yield()
{
  assert(sc.running);
  char dummy = 0;
  // 栈未被消耗完
  assert(sc.stack <= &dummy);
  stack_.resize(sc.stack + STACK_SIZE - &dummy);
  memcpy(stack_.data(), &dummy, stack_.size());
  status_ = StatusEnum::COROUTINE_SUSPEND;
  sc.running = nullptr;
  swapcontext(&ctx_, &sc.main);
}

void
Coroutine::mainfunc(uint32_t low32, uint32_t hi32)
{
  uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
  Coroutine* co = (Coroutine*)ptr;
  co->func_(co);
  co->status_ = StatusEnum::COROUTINE_DEAD;
  sc.running = nullptr;
}

void
co_yield()
{
  sc.running->yield();
}

void
co_resume(int id)
{
  assert(id < sc.cos.size());
  assert(sc.cos[id]);
  sc.cos[id]->resume();
}

int
co_id()
{
  assert(sc.running);
  return sc.running->id();
}
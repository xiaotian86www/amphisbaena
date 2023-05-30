#include "coroutine.hpp"

#include <cstdint>
#include <cassert>
#include <cstring>

Coroutine::Coroutine(Schedule *s, int id, Coroutine::coroutine_func &&func)
    : func_(std::move(func)),
      sch_(s),
      status_(StatusEnum::COROUTINE_READY),
      id_(id)
{
}

Coroutine::~Coroutine()
{
}

void Coroutine::resume()
{
    switch (status_)
    {
    case StatusEnum::COROUTINE_READY:
    {
        getcontext(&ctx_); // 只是为了获取帧结构，以下动作完善帧结构
        ctx_.uc_stack.ss_sp = sch_->stack_;
        ctx_.uc_stack.ss_size = STACK_SIZE;
        ctx_.uc_link = &sch_->main_;
        // sch_->running_ = this;
        status_ = StatusEnum::COROUTINE_RUNNING;
        uintptr_t ptr = (uintptr_t)this;
        // mainfunc只支持int类型参数，不支持void*参数属于设计问题
        makecontext(&ctx_, (void (*)(void))mainfunc, 2, (uint32_t)ptr, (uint32_t)(ptr >> 32));
        swapcontext(&sch_->main_, &ctx_); // 保存主线程帧，设置协程帧为当前帧
        break;
    }
    case StatusEnum::COROUTINE_SUSPEND:
    {
        memcpy(sch_->stack_ + STACK_SIZE - stack_.size(), stack_.data(), stack_.size());
        // sch_->running_ = this;
        status_ = StatusEnum::COROUTINE_RUNNING;
        swapcontext(&sch_->main_, &ctx_);
        break;
    }
    default:
        assert(0);
    }
}

void Coroutine::yield()
{
    char dummy = 0;
    // 栈未被消耗完
    assert(sch_->stack_ <= &dummy);
    stack_.resize(sch_->stack_ + STACK_SIZE - &dummy);
    memcpy(stack_.data(), &dummy, stack_.size());
    status_ = StatusEnum::COROUTINE_SUSPEND;
    // sch_->running_ = nullptr;
    swapcontext(&ctx_, &sch_->main_);
}

void Coroutine::mainfunc(uint32_t low32, uint32_t hi32)
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    Coroutine *co = (Coroutine *)ptr;
    co->func_(co);
    co->status_ = StatusEnum::COROUTINE_DEAD;
    // co->sch_->running_ = nullptr;
}

Schedule::Schedule()
{
}

Schedule::~Schedule()
{
}

std::unique_ptr<Coroutine> Schedule::create(Coroutine::coroutine_func &&func)
{
    return std::make_unique<Coroutine>(this, max_co_id_++, std::move(func));
}

// std::unique_ptr<Coroutine> Schedule::running()
// {
//     return running_;
// }
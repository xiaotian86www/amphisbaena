#include "coroutine.hpp"

#include <cstdint>
#include <cassert>
#include <cstring>

Coroutine::Coroutine(Schedule *s, coroutine_func func, void *ud)
    : func_(func), ud_(ud), sch_(s), status_(COROUTINE_READY)
{
}

Coroutine::~Coroutine()
{
}

void Coroutine::resume()
{
    switch (status_)
    {
    case COROUTINE_READY:
    {
        getcontext(&ctx_); // 只是为了获取帧结构，以下动作完善帧结构
        ctx_.uc_stack.ss_sp = sch_->stack_;
        ctx_.uc_stack.ss_size = STACK_SIZE;
        ctx_.uc_link = &sch_->main_;
        sch_->running_ = this;
        status_ = COROUTINE_RUNNING;
        uintptr_t ptr = (uintptr_t)this;
        // mainfunc只支持int类型参数，不支持void*参数属于设计问题
        makecontext(&ctx_, (void (*)(void))mainfunc, 2, (uint32_t)ptr, (uint32_t)(ptr >> 32));
        swapcontext(&sch_->main_, &ctx_); // 保存主线程帧，设置协程帧为当前帧
        break;
    }
    case COROUTINE_SUSPEND:
    {
        memcpy(sch_->stack_ + STACK_SIZE - stack_.size(), stack_.data(), stack_.size());
        sch_->running_ = this;
        status_ = COROUTINE_RUNNING;
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
    status_ = COROUTINE_SUSPEND;
    sch_->running_ = nullptr;
    swapcontext(&ctx_, &sch_->main_);
}

void Coroutine::mainfunc(uint32_t low32, uint32_t hi32)
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    Coroutine *co = (Coroutine *)ptr;
    co->func_(co->ud_);
    // TODO 删除co
    co->sch_->running_ = nullptr;
}

Schedule::Schedule()
    : running_(nullptr)
{
}

Schedule::~Schedule()
{
}

Coroutine *Schedule::create(coroutine_func func, void *ud)
{
    return &cos_.emplace_back(this, func, ud);
}

Coroutine *Schedule::running()
{
    return running_;
}
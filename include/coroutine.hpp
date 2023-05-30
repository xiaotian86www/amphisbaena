#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <list>
#include <vector>
#include <memory>
#include <ucontext.h>

#define STACK_SIZE (1024 * 1024)
#define DEFAULT_COROUTINE 16

// #define COROUTINE_DEAD 0
// #define COROUTINE_READY 1
// #define COROUTINE_RUNNING 2
// #define COROUTINE_SUSPEND 3

class Schedule;

class Coroutine;

class Coroutine
{
public:
    enum class StatusEnum : int
    {
        COROUTINE_DEAD = 0,
        COROUTINE_READY = 1,
        COROUTINE_RUNNING = 2,
        COROUTINE_SUSPEND = 3
    };

    using coroutine_func = std::function<void(Coroutine *)>;

public:
    Coroutine(Schedule *s, int id, coroutine_func &&func);

    ~Coroutine();

public:
    void resume();

    void yield();

public:
    StatusEnum status()
    {
        return status_;
    }

    int id()
    {
        return id_;
    }

private:
    static void mainfunc(uint32_t low32, uint32_t hi32);

private:
    std::vector<char> stack_;
    coroutine_func func_;
    ucontext_t ctx_;
    Schedule *sch_;
    StatusEnum status_;
    int id_;
};

class Schedule
{
    friend class Coroutine;

public:
    Schedule();
    ~Schedule();

public:
    std::unique_ptr<Coroutine> create(Coroutine::coroutine_func &&func);

private:
    char stack_[STACK_SIZE];
    ucontext_t main_;
    int max_co_id_ = 0;
};

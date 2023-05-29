#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <vector>
#include <ucontext.h>

#define STACK_SIZE (1024 * 1024)
#define DEFAULT_COROUTINE 16

#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3

typedef void (*coroutine_func)(void *ud);

class Schedule;

class Coroutine
{
public:
    Coroutine(Schedule *s , coroutine_func func, void *ud);

    ~Coroutine();

public:
    void resume();

    void yield();

private:
    static void mainfunc(uint32_t low32, uint32_t hi32);

private:
    coroutine_func func_;
    void *ud_;
    ucontext_t ctx_;
    Schedule *sch_;
    int status_;
    std::vector<char> stack_;
};

class Schedule
{
    friend class Coroutine;
public:
    Schedule();
    ~Schedule();

public:
    Coroutine *create(coroutine_func, void *ud);

    Coroutine *running();

private:
    char stack_[STACK_SIZE];
    ucontext_t main_;
    Coroutine *running_;
    std::list<Coroutine> cos_;
};

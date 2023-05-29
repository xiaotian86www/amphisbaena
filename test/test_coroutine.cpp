#include <gtest/gtest.h>

#include "coroutine.hpp"

struct args
{
    int n;
};

void foo(Coroutine *co, void *arg)
{
    args *arg_ = (args *)arg;
    for (int i = 0; i < 5; i++)
    {
        printf("coroutine %d : %d\n", co->id(), arg_->n + i);
        co->yield();
    }
}

TEST(coroutine, test1)
{
    Schedule s;
    args a1 = {1};
    args a2 = {21};
    auto c1 = s.create(foo, &a1);
    auto c2 = s.create(foo, &a2);

    printf("main start\n");
    while (c1->status() != Coroutine::StatusEnum::COROUTINE_DEAD &&
           c2->status() != Coroutine::StatusEnum::COROUTINE_DEAD)
    {
        c1->resume();
        c2->resume();
    }
    printf("main end\n");
}
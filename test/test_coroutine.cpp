#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "coroutine.hpp"

struct args
{
    int n;
};

void foo(std::function<void(int)> func)
{
    for (int i = 0; i < 2; i++)
    {
        func(co_id() * 10 + i);
        co_yield();
    }
}

TEST(coroutine, test1)
{
    testing::MockFunction<void(int)> foo_mock;
    testing::Sequence dummy;
    EXPECT_CALL(foo_mock, Call(0));
    EXPECT_CALL(foo_mock, Call(10));
    EXPECT_CALL(foo_mock, Call(1));
    EXPECT_CALL(foo_mock, Call(11));
    Coroutine c1(std::bind(foo, foo_mock.AsStdFunction()));
    Coroutine c2(std::bind(foo, foo_mock.AsStdFunction()));

    while (c1.status() != Coroutine::StatusEnum::COROUTINE_DEAD &&
           c2.status() != Coroutine::StatusEnum::COROUTINE_DEAD)
    {
        c1.resume();
        c2.resume();
    }
}
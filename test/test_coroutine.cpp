#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "coroutine.hpp"

struct args
{
    int n;
};

void foo(Coroutine *co, std::function<void(int)> func)
{
    for (int i = 0; i < 2; i++)
    {
        func(co->id() * 10 + i);
        co->yield();
    }
}

TEST(coroutine, test1)
{
    testing::MockFunction<void(int)> foo_mock;
    Schedule s;
    testing::Sequence dummy;
    EXPECT_CALL(foo_mock, Call(0));
    EXPECT_CALL(foo_mock, Call(10));
    EXPECT_CALL(foo_mock, Call(1));
    EXPECT_CALL(foo_mock, Call(11));
    auto c1 = s.create(std::bind(foo, std::placeholders::_1, foo_mock.AsStdFunction()));
    auto c2 = s.create(std::bind(foo, std::placeholders::_1, foo_mock.AsStdFunction()));

    while (c1->status() != Coroutine::StatusEnum::COROUTINE_DEAD &&
           c2->status() != Coroutine::StatusEnum::COROUTINE_DEAD)
    {
        c1->resume();
        c2->resume();
    }
}
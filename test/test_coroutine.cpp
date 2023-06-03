#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "coroutine.hpp"

struct args
{
  int n;
};

void
foo(std::function<void(int)> func)
{
  for (int i = 0; i < 2; i++) {
    func(translator::co_id() * 10 + i);
    translator::co_yield ();
  }
}

TEST(coroutine, test1)
{
  testing::MockFunction<void(int)> foo_mock;
  translator::Schedule sch;
  auto c1 = sch.create(std::bind(foo, foo_mock.AsStdFunction()));
  auto c2 = sch.create(std::bind(foo, foo_mock.AsStdFunction()));

  testing::Sequence dummy;
  EXPECT_CALL(foo_mock, Call(c1->id() * 10));
  EXPECT_CALL(foo_mock, Call(c2->id() * 10));
  EXPECT_CALL(foo_mock, Call(c1->id() * 10 + 1));
  EXPECT_CALL(foo_mock, Call(c2->id() * 10 + 1));

  while (c1->status() != translator::Coroutine::StatusEnum::COROUTINE_DEAD &&
         c2->status() != translator::Coroutine::StatusEnum::COROUTINE_DEAD) {
    translator::co_resume(c1->id());
    translator::co_resume(c2->id());
  }
}

/**
 * @brief 检查是否复用id
 * 
 */
TEST(coroutine, test2)
{
  testing::MockFunction<void(int)> foo_mock;
  translator::Schedule sch;
  {
    auto c1 = sch.create(std::bind(foo, foo_mock.AsStdFunction()));
    EXPECT_EQ(c1->id(), 0);
    auto c2 = sch.create(std::bind(foo, foo_mock.AsStdFunction()));
    EXPECT_EQ(c2->id(), 1);
  }
  {
    auto c1 = sch.create(std::bind(foo, foo_mock.AsStdFunction()));
    EXPECT_EQ(c1->id(), 0);
    auto c2 = sch.create(std::bind(foo, foo_mock.AsStdFunction()));
    EXPECT_EQ(c2->id(), 1);
  }
}
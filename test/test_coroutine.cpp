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
    translator::Schedule* sch = translator::Schedule::current_schedule();
    func(sch->running_id() * 10 + i);
    sch->yield();
  }
}

TEST(coroutine, test1)
{
  testing::MockFunction<void(int)> foo_mock;
  translator::Schedule sch;
  int c1 = sch.create(std::bind(foo, foo_mock.AsStdFunction()));
  int c2 = sch.create(std::bind(foo, foo_mock.AsStdFunction()));
  sch.resume(c1);
  sch.resume(c2);

  testing::Sequence dummy;
  EXPECT_CALL(foo_mock, Call(c1 * 10));
  EXPECT_CALL(foo_mock, Call(c2 * 10));
  EXPECT_CALL(foo_mock, Call(c1 * 10 + 1));
  EXPECT_CALL(foo_mock, Call(c2 * 10 + 1));
  
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

/**
 * @brief 检查是否复用id
 *
 */
TEST(coroutine, test2)
{
  testing::MockFunction<void(int)> foo_mock;
  translator::Schedule sch;

  int c1 = sch.create([] {});
  EXPECT_EQ(c1, 0);
  int c2 = sch.create([] {});
  EXPECT_EQ(c2, 1);
  auto c3 = sch.create([]{});
  EXPECT_EQ(c3, 2);
  auto c4 = sch.create([]{});
  EXPECT_EQ(c4, 3);
}
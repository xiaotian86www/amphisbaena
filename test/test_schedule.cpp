#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "schedule.hpp"

struct args
{
  int n;
};

static void
foo(std::function<void(int)> func)
{
  for (int i = 0; i < 2; i++) {
    translator::Schedule* sch = translator::Schedule::this_sch();
    func(0);
    sch->resume(sch->this_co());
    sch->yield();
  }
}

TEST(coroutine, test1)
{
  testing::MockFunction<void(int)> foo_mock;
  testing::Sequence dummy;
  EXPECT_CALL(foo_mock, Call(0)).Times(4);

  translator::Schedule sch;
  sch.post(std::bind(foo, foo_mock.AsStdFunction()));
  sch.post(std::bind(foo, foo_mock.AsStdFunction()));
}

// /**
//  * @brief 检查是否复用id
//  *
//  */
// TEST(coroutine, test2)
// {
//   testing::MockFunction<void(int)> foo_mock;
//   translator::Schedule sch;

//   int c1 = sch.post([] {});
//   EXPECT_EQ(c1, 0);
//   int c2 = sch.post([] {});
//   EXPECT_EQ(c2, 1);
//   auto c3 = sch.post([]{});
//   EXPECT_EQ(c3, 2);
//   auto c4 = sch.post([]{});
//   EXPECT_EQ(c4, 3);
// }
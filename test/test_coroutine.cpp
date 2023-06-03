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
    translator::Schedule* sch = translator::Schedule::this_sch();
    func(sch->this_co()->id * 10 + i);
    sch->yield();
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
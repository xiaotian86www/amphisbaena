
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>

#include "future.hpp"

static void
foo(std::function<void(translator::Promise<int>&)> func1,
    std::function<void(int)> func2)
{
  translator::Promise<int> pms;
  func1(pms);
  auto& ftr = pms.future();
  func2(ftr.get());
}

TEST(future, test_1)
{
  testing::MockFunction<void(translator::Promise<int>&)> foo_mock1;
  testing::MockFunction<void(int)> foo_mock2;

  EXPECT_CALL(foo_mock1, Call(testing::_))
    .Times(2)
    .WillRepeatedly(
      testing::Invoke([](translator::Promise<int>& pms) { pms.set(0); }));
  EXPECT_CALL(foo_mock2, Call(0)).Times(2);

  auto sch = std::make_shared<translator::Schedule>();
  sch->post(
    std::bind(foo, foo_mock1.AsStdFunction(), foo_mock2.AsStdFunction()));
  sch->post(
    std::bind(foo, foo_mock1.AsStdFunction(), foo_mock2.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch.get()));

  // sch->stop();

  if (th.joinable())
    th.join();
}
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>

#include "schedule.hpp"

struct args
{
  int n;
};

static void
foo(translator::ScheduleRef sch, std::function<void(int)> func)
{
  for (int i = 0; i < 2; i++) {
    func(i);
    sch.resume(sch.this_co());
    sch.yield();
  }
}

TEST(coroutine, test1)
{
  testing::MockFunction<void(int)> foo_mock;
  EXPECT_CALL(foo_mock, Call(0)).Times(2);
  EXPECT_CALL(foo_mock, Call(1)).Times(2);

  auto sch = std::make_shared<translator::Schedule>();
  sch->post(std::bind(foo, std::placeholders::_1, foo_mock.AsStdFunction()));
  sch->post(std::bind(foo, std::placeholders::_1, foo_mock.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch.get()));

  if (th.joinable())
    th.join();
}

/**
 * @brief 中途停止
 * 
 */
TEST(coroutine, test2)
{
  testing::MockFunction<void(int)> foo_mock;
  
  auto sch = std::make_shared<translator::Schedule>();
  
  EXPECT_CALL(foo_mock, Call(0)).WillOnce(testing::Invoke([sch](int){
    sch->stop();
  }));

  sch->post(std::bind(foo, std::placeholders::_1, foo_mock.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch.get()));

  if (th.joinable())
    th.join();
}
#include <bits/types/struct_timespec.h>
#include <chrono>
#include <ctime>
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
    // TODO 这种写法不合理，但为了达到自我唤醒效果先如此实现
    sch.resume(sch.this_co());
    sch.yield();
  }
}

static void
foo2(translator::ScheduleRef sch, std::function<void(int)> func)
{
  for (int i = 0; i < 2; i++) {
    func(i);
    sch.yield_for(std::chrono::milliseconds(1));
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

  EXPECT_CALL(foo_mock, Call(0)).WillOnce(testing::Invoke([sch](int) {
    sch->stop();
  }));

  sch->post(std::bind(foo, std::placeholders::_1, foo_mock.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch.get()));

  if (th.joinable())
    th.join();
}

/**
 * @brief yield_for
 *
 */
TEST(coroutine, test3)
{
  testing::MockFunction<void(int)> foo_mock;

  auto sch = std::make_shared<translator::Schedule>();

  EXPECT_CALL(foo_mock, Call(0));
  EXPECT_CALL(foo_mock, Call(1));

  sch->post(std::bind(foo2, std::placeholders::_1, foo_mock.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch.get()));

  timespec start, stop;
  clock_gettime(CLOCK_MONOTONIC, &start);
  if (th.joinable())
    th.join();
  clock_gettime(CLOCK_MONOTONIC, &stop);

  EXPECT_GT((stop.tv_sec - start.tv_sec) * 1000000000 +
            (stop.tv_nsec - start.tv_nsec), 1000000 * 2);
}

/**
 * @brief yield_for 不等到超时时间
 * 
 */
TEST(coroutine, test4)
{

}
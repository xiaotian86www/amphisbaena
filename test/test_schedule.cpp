#include <chrono>
#include <ctime>
#include <thread>

#include "schedule.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

struct args
{
  int n;
};

static void
foo(translator::ScheduleRef sch,
    std::function<void(translator::ScheduleRef, int)> func)
{
  for (int i = 0; i < 2; i++) {
    func(sch, i);
  }
}

TEST(coroutine, test1)
{
  testing::MockFunction<void(translator::ScheduleRef, int)> foo_mock;
  auto invoke_foo = [](translator::ScheduleRef sch, int) {
    sch.resume(sch.this_co());
    sch.yield();
  };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, 0))
    .Times(2)
    .WillRepeatedly(testing::Invoke(invoke_foo));
  EXPECT_CALL(foo_mock, Call(testing::_, 1))
    .Times(2)
    .WillRepeatedly(testing::Invoke(invoke_foo));

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
  testing::MockFunction<void(translator::ScheduleRef, int)> foo_mock;
  auto invoke_foo = [](translator::ScheduleRef sch, int) {
    sch.stop();
    sch.resume(sch.this_co());
    sch.yield();
  };

  EXPECT_CALL(foo_mock, Call(testing::_, 0))
    .WillOnce(testing::Invoke(invoke_foo));

  auto sch = std::make_shared<translator::Schedule>();
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
  testing::MockFunction<void(translator::ScheduleRef, int)> foo_mock;
  auto invoke_foo = [](translator::ScheduleRef sch, int) {
    sch.yield_for(std::chrono::milliseconds(1));
  };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, 0))
    .WillOnce(testing::Invoke(invoke_foo));
  EXPECT_CALL(foo_mock, Call(testing::_, 1))
    .WillOnce(testing::Invoke(invoke_foo));

  auto sch = std::make_shared<translator::Schedule>();
  sch->post(std::bind(foo, std::placeholders::_1, foo_mock.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch.get()));

  timespec start, stop;
  clock_gettime(CLOCK_MONOTONIC, &start);
  if (th.joinable())
    th.join();
  clock_gettime(CLOCK_MONOTONIC, &stop);

  EXPECT_GT((stop.tv_sec - start.tv_sec) * 1000000000 +
              (stop.tv_nsec - start.tv_nsec),
            1000000 * 2);
}

/**
 * @brief yield_for 不等到超时时间
 *
 */
TEST(coroutine, test4) {}
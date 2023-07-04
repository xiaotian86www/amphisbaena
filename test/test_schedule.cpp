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
    translator::Coroutine co,
    std::function<void(translator::Coroutine, int)> func)
{
  for (int i = 0; i < 2; i++) {
    func(co, i);
  }
}

TEST(coroutine, resume)
{
  testing::MockFunction<void(translator::Coroutine, int)> foo_mock;
  auto invoke_foo = [](translator::Coroutine co, int) {
    co.resume();
    co.yield();
  };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, 0))
    .Times(2)
    .WillRepeatedly(testing::Invoke(invoke_foo));
  EXPECT_CALL(foo_mock, Call(testing::_, 1))
    .Times(2)
    .WillRepeatedly(testing::Invoke(invoke_foo));

  auto sch = std::make_shared<translator::Schedule>();
  sch->spawn(std::bind(foo,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock.AsStdFunction()));
  sch->spawn(std::bind(foo,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch));

  if (th.joinable())
    th.join();
}

TEST(coroutine, multi_resume)
{
  testing::MockFunction<void(translator::Coroutine, int)> foo_mock;
  auto invoke_foo = [](translator::Coroutine co, int) {
    co.resume();
    co.resume();
    co.resume();
    co.yield();
  };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, 0))
    .Times(2)
    .WillRepeatedly(testing::Invoke(invoke_foo));
  EXPECT_CALL(foo_mock, Call(testing::_, 1))
    .Times(2)
    .WillRepeatedly(testing::Invoke(invoke_foo));

  auto sch = std::make_shared<translator::Schedule>();
  sch->spawn(std::bind(foo,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock.AsStdFunction()));
  sch->spawn(std::bind(foo,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch));

  if (th.joinable())
    th.join();
}

/**
 * @brief 中途停止
 *
 */
TEST(coroutine, stop)
{
  auto sch = std::make_shared<translator::Schedule>();

  testing::MockFunction<void(translator::Coroutine, int)> foo_mock;
  auto invoke_foo = [sch](translator::Coroutine co, int) {
    sch->stop();
    co.resume();
    co.yield();
  };

  EXPECT_CALL(foo_mock, Call(testing::_, 0))
    .WillOnce(testing::Invoke(invoke_foo));

  sch->spawn(std::bind(foo,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch));

  if (th.joinable())
    th.join();
}

/**
 * @brief yield_for
 *
 */
TEST(coroutine, yield_for_timeout)
{
  testing::MockFunction<void(translator::Coroutine, int)> foo_mock;
  auto invoke_foo = [](translator::Coroutine co, int) { co.yield_for(1); };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, 0))
    .WillOnce(testing::Invoke(invoke_foo));
  EXPECT_CALL(foo_mock, Call(testing::_, 1))
    .WillOnce(testing::Invoke(invoke_foo));

  auto sch = std::make_shared<translator::Schedule>();
  sch->spawn(std::bind(foo,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch));

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
 * @brief yield_for
 *
 */
TEST(coroutine, resume_yield_for)
{
  testing::MockFunction<void(translator::Coroutine, int)> foo_mock;
  auto invoke_foo = [](translator::Coroutine co, int) {
    co.resume();
    co.yield_for(1);
  };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, 0))
    .WillOnce(testing::Invoke(invoke_foo));
  EXPECT_CALL(foo_mock, Call(testing::_, 1))
    .WillOnce(testing::Invoke(invoke_foo));

  auto sch = std::make_shared<translator::Schedule>();
  sch->spawn(std::bind(foo,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch));

  timespec start, stop;
  clock_gettime(CLOCK_MONOTONIC, &start);
  if (th.joinable())
    th.join();
  clock_gettime(CLOCK_MONOTONIC, &stop);

  EXPECT_LT((stop.tv_sec - start.tv_sec) * 1000000000 +
              (stop.tv_nsec - start.tv_nsec),
            1000000 * 2);
}

/**
 * @brief yield_for 不等到超时时间
 *
 */
TEST(coroutine, stop_yield_for)
{
  auto sch = std::make_shared<translator::Schedule>();

  testing::MockFunction<void(translator::Coroutine, int)> foo_mock;

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, 0))
    .WillOnce(testing::Invoke(
      [](translator::Coroutine co, int) { co.yield_for(10); }))
    .WillOnce(testing::Invoke([sch](translator::Coroutine co, int) {
      sch->stop();
      co.yield();
    }));

  sch->spawn(std::bind(foo,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock.AsStdFunction()));
  sch->spawn(std::bind(foo,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch));

  timespec start, stop;
  clock_gettime(CLOCK_MONOTONIC, &start);
  if (th.joinable())
    th.join();
  clock_gettime(CLOCK_MONOTONIC, &stop);

  EXPECT_LT((stop.tv_sec - start.tv_sec) * 1000000000 +
              (stop.tv_nsec - start.tv_nsec),
            10000000);
}

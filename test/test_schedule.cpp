#include <boost/asio/io_service.hpp>
#include <chrono>
#include <ctime>
#include <memory>
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
    translator::CoroutineRef co,
    std::function<void(translator::ScheduleRef, translator::CoroutineRef, int)>
      func)
{
  for (int i = 0; i < 2; i++) {
    func(sch, co, i);
  }
}

class Coroutine : public testing::Test
{
public:
  virtual void SetUp() { sch = std::make_shared<translator::Schedule>(ios); }

  virtual void TearDown() {}

protected:
  boost::asio::io_service ios;
  std::shared_ptr<translator::Schedule> sch;
};

TEST_F(Coroutine, resume)
{
  testing::MockFunction<void(
    translator::ScheduleRef, translator::CoroutineRef, int)>
    foo_mock;
  auto invoke_foo =
    [](translator::ScheduleRef sch, translator::CoroutineRef co, int) {
      sch.resume(co);
      co.yield();
    };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 0))
    .Times(2)
    .WillRepeatedly(testing::Invoke(invoke_foo));
  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 1))
    .Times(2)
    .WillRepeatedly(testing::Invoke(invoke_foo));

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

TEST_F(Coroutine, multi_resume)
{
  testing::MockFunction<void(
    translator::ScheduleRef, translator::CoroutineRef, int)>
    foo_mock;
  auto invoke_foo =
    [](translator::ScheduleRef sch, translator::CoroutineRef co, int) {
      sch.resume(co);
      sch.resume(co);
      sch.resume(co);
      co.yield();
    };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 0))
    .Times(2)
    .WillRepeatedly(testing::Invoke(invoke_foo));
  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 1))
    .Times(2)
    .WillRepeatedly(testing::Invoke(invoke_foo));

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
TEST_F(Coroutine, stop)
{
  testing::MockFunction<void(
    translator::ScheduleRef, translator::CoroutineRef, int)>
    foo_mock;
  auto invoke_foo =
    [this](translator::ScheduleRef, translator::CoroutineRef co, int) {
      sch->stop();
      sch->resume(co);
      co.yield();
    };

  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 0))
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
TEST_F(Coroutine, yield_for_timeout)
{
  testing::MockFunction<void(
    translator::ScheduleRef, translator::CoroutineRef, int)>
    foo_mock;
  auto invoke_foo = [](translator::ScheduleRef sch,
                       translator::CoroutineRef co,
                       int) { co.yield_for(1); };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 0))
    .WillOnce(testing::Invoke(invoke_foo));
  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 1))
    .WillOnce(testing::Invoke(invoke_foo));

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
TEST_F(Coroutine, resume_yield_for)
{
  testing::MockFunction<void(
    translator::ScheduleRef, translator::CoroutineRef, int)>
    foo_mock;
  auto invoke_foo =
    [](translator::ScheduleRef sch, translator::CoroutineRef co, int) {
      sch.resume(co);
      co.yield_for(1);
    };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 0))
    .WillOnce(testing::Invoke(invoke_foo));
  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 1))
    .WillOnce(testing::Invoke(invoke_foo));

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
TEST_F(Coroutine, stop_yield_for)
{
  testing::MockFunction<void(
    translator::ScheduleRef, translator::CoroutineRef, int)>
    foo_mock;

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 0))
    .WillOnce(testing::Invoke([](translator::ScheduleRef sch,
                                 translator::CoroutineRef co,
                                 int) { co.yield_for(10); }))
    .WillOnce(testing::Invoke(
      [this](translator::ScheduleRef, translator::CoroutineRef co, int) {
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

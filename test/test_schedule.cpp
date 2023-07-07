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

class Coroutine : public testing::Test
{
public:
  Coroutine()
    : sch(std::make_shared<translator::Schedule>(ios))
  {
  }

  ~Coroutine()
  {
    if (th.joinable())
      th.join();
  }

public:
  virtual void SetUp() {}

  virtual void TearDown() {}

public:
  void foo(
    translator::ScheduleRef sch,
    translator::CoroutineRef co)
  {
    for (int i = 0; i < 2; i++) {
      foo_mock.AsStdFunction()(sch, co, i);
    }
  }

protected:
  boost::asio::io_service ios;
  std::thread th;
  std::shared_ptr<translator::Schedule> sch;

  testing::MockFunction<
    void(translator::ScheduleRef, translator::CoroutineRef, int)>
    foo_mock;
};

TEST_F(Coroutine, resume)
{
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

  sch->spawn(std::bind(&Coroutine::foo, this,
                       std::placeholders::_1,
                       std::placeholders::_2));
  sch->spawn(std::bind(&Coroutine::foo, this,
                       std::placeholders::_1,
                       std::placeholders::_2));

  th = std::thread(std::bind(&translator::Schedule::run, sch));
}

TEST_F(Coroutine, multi_resume)
{
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

  sch->spawn(std::bind(&Coroutine::foo, this,
                       std::placeholders::_1,
                       std::placeholders::_2));
  sch->spawn(std::bind(&Coroutine::foo, this,
                       std::placeholders::_1,
                       std::placeholders::_2));

  th = std::thread(std::bind(&translator::Schedule::run, sch));
}

/**
 * @brief 中途停止
 *
 */
TEST_F(Coroutine, stop)
{
  auto invoke_foo =
    [this](translator::ScheduleRef, translator::CoroutineRef co, int) {
      sch->stop();
      sch->resume(co);
      co.yield();
    };

  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 0))
    .WillOnce(testing::Invoke(invoke_foo));

  sch->spawn(std::bind(&Coroutine::foo, this,
                       std::placeholders::_1,
                       std::placeholders::_2));

  th = std::thread(std::bind(&translator::Schedule::run, sch));
}

/**
 * @brief yield_for
 *
 */
TEST_F(Coroutine, yield_for_timeout)
{
  auto invoke_foo =
    [](translator::ScheduleRef sch, translator::CoroutineRef co, int) {
      timespec start, stop;
      clock_gettime(CLOCK_MONOTONIC, &start);
      co.yield_for(1);
      clock_gettime(CLOCK_MONOTONIC, &stop);

      EXPECT_GT((stop.tv_sec - start.tv_sec) * 1000000000 +
                  (stop.tv_nsec - start.tv_nsec),
                1000000);
    };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 0))
    .WillOnce(testing::Invoke(invoke_foo));
  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 1))
    .WillOnce(testing::Invoke(invoke_foo));

  sch->spawn(std::bind(&Coroutine::foo, this,
                       std::placeholders::_1,
                       std::placeholders::_2));

  th = std::thread(std::bind(&translator::Schedule::run, sch));
}

/**
 * @brief yield_for
 *
 */
TEST_F(Coroutine, resume_yield_for)
{
  auto invoke_foo =
    [](translator::ScheduleRef sch, translator::CoroutineRef co, int) {
      sch.resume(co);
      timespec start, stop;
      clock_gettime(CLOCK_MONOTONIC, &start);
      co.yield_for(1);
      clock_gettime(CLOCK_MONOTONIC, &stop);

      EXPECT_LT((stop.tv_sec - start.tv_sec) * 1000000000 +
                  (stop.tv_nsec - start.tv_nsec),
                1000000);
    };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 0))
    .WillOnce(testing::Invoke(invoke_foo));
  EXPECT_CALL(foo_mock, Call(testing::_, testing::_, 1))
    .WillOnce(testing::Invoke(invoke_foo));

  sch->spawn(std::bind(&Coroutine::foo, this,
                       std::placeholders::_1,
                       std::placeholders::_2));

  th = std::thread(std::bind(&translator::Schedule::run, sch));
}

/**
 * @brief yield_for 不等到超时时间
 *
 */
TEST_F(Coroutine, stop_yield_for)
{
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

  sch->spawn(std::bind(&Coroutine::foo, this,
                       std::placeholders::_1,
                       std::placeholders::_2));
  sch->spawn(std::bind(&Coroutine::foo, this,
                       std::placeholders::_1,
                       std::placeholders::_2));

  th = std::thread(std::bind(&translator::Schedule::run, sch));
}

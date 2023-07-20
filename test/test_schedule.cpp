#include "gtest/gtest.h"
#include <boost/asio/io_service.hpp>
#include <chrono>
#include <ctime>
#include <exception>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include "fixture/fixture_schedule.hpp"
#include "schedule.hpp"
#include "tool/util.hpp"

class Coroutine : public FixtureSchedule
{
protected:
  testing::MockFunction<void(int)> foo_mock;
};

TEST_F(Coroutine, resume)
{
  auto foo = [this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    for (int i = 0; i < 2; i++) {
      sch.resume(co);
      co.yield();
      foo_mock.Call(i);
    }
  };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(0)).Times(2);
  EXPECT_CALL(foo_mock, Call(1)).Times(2);

  sch->spawn(foo);
  sch->spawn(foo);
}

TEST_F(Coroutine, multi_resume)
{
  auto foo = [this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    sch.resume(co);
    sch.resume(co);
    sch.resume(co);
    co.yield();
    foo_mock.Call(0);
  };

  testing::Sequence seq;
  EXPECT_CALL(foo_mock, Call(testing::_)).Times(2);

  sch->spawn(foo);
  sch->spawn(foo);
}

/**
 * @brief 中途停止
 *
 */
TEST_F(Coroutine, stop)
{
  auto foo = [this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    stop();
    foo_mock.Call(0);
    sch.resume(co);
    foo_mock.Call(1);
    co.yield();
    foo_mock.Call(2);
  };

  EXPECT_CALL(foo_mock, Call(testing::Lt(2))).Times(2);

  sch->spawn(foo);
}

/**
 * @brief 触发异常
 *
 */
TEST_F(Coroutine, exception)
{
  auto foo = [this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    sch.resume(co);
    foo_mock.Call(0);
    co.yield();
    throw std::exception();
    foo_mock.Call(1);
  };

  EXPECT_CALL(foo_mock, Call(testing::Lt(1))).Times(1);

  sch->spawn(foo);
}

/**
 * @brief yield_for
 *
 */
TEST_F(Coroutine, yield_for_timeout)
{
  auto foo = [this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    EXPECT_SPEND_GT(co.yield_for(1), 1000000);
    foo_mock.Call(0);
  };

  EXPECT_CALL(foo_mock, Call(testing::_)).Times(1);

  sch->spawn(foo);
}

/**
 * @brief yield_for
 *
 */
TEST_F(Coroutine, resume_yield_for)
{
  auto foo = [this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    sch.resume(co);
    EXPECT_SPEND_LT(co.yield_for(1), 1000000);
    foo_mock.Call(0);
  };

  EXPECT_CALL(foo_mock, Call(testing::_)).Times(1);

  sch->spawn(foo);
}

/**
 * @brief yield_for 不等到超时时间
 *
 */
TEST_F(Coroutine, stop_yield_for)
{
  auto foo1 = [this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    foo_mock.Call(0);
    co.yield_for(10);
    foo_mock.Call(2);
  };

  auto foo2 = [this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    foo_mock.Call(1);
    stop();
    co.yield();
    foo_mock.Call(3);
  };

  EXPECT_CALL(foo_mock, Call(testing::Lt(2))).Times(2);

  sch->spawn(foo1);
  sch->spawn(foo2);
}

#include <chrono>
#include <memory>
#include <thread>

#include "detail/asio_schedule.hpp"
#include "future.hpp"
#include "schedule.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

static void
foo(translator::ScheduleRef sch,
    translator::CoroutineRef co,
    std::function<int()> func1,
    std::function<void(int)> func2)
{
  translator::Promise<int> pms(co);
  pms.set(func1());
  func2(pms.future().get());
}

static void
foo2(translator::ScheduleRef sch,
     translator::CoroutineRef co,
     std::function<int()> func1,
     std::function<void(int)> func2)
{
  // translator::Promise<int> pms(sch);
  // pms.set(func1());
  // func2(pms.future().get_for(0, 1));
  co.yield_for(0);
  func2(func1());
}

static void
foo3(translator::ScheduleRef sch,
     translator::CoroutineRef co,
     std::function<int()> func1,
     std::function<void(int)> func2)
{
  translator::Promise<int> pms(co);
  func2(pms.future().get_for(1, 0));
}

TEST(future, get)
{
  testing::MockFunction<int()> foo_mock1;
  testing::MockFunction<void(int)> foo_mock2;

  EXPECT_CALL(foo_mock1, Call()).Times(2).WillRepeatedly(testing::Return(1));
  EXPECT_CALL(foo_mock2, Call(1)).Times(2);

  auto sch = std::make_shared<translator::AsioSchedule>();
  sch->spawn(std::bind(foo,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock1.AsStdFunction(),
                       foo_mock2.AsStdFunction()));
  sch->spawn(std::bind(foo,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock1.AsStdFunction(),
                       foo_mock2.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch.get()));

  if (th.joinable())
    th.join();
}

TEST(future, get_for)
{
  testing::MockFunction<int()> foo_mock1;
  testing::MockFunction<void(int)> foo_mock2;

  EXPECT_CALL(foo_mock1, Call()).Times(2).WillRepeatedly(testing::Return(1));
  EXPECT_CALL(foo_mock2, Call(1)).Times(2);

  auto sch = std::make_shared<translator::AsioSchedule>();
  sch->spawn(std::bind(foo2,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock1.AsStdFunction(),
                       foo_mock2.AsStdFunction()));
  sch->spawn(std::bind(foo2,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock1.AsStdFunction(),
                       foo_mock2.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch.get()));

  if (th.joinable())
    th.join();
}

TEST(future, get_for_timeout)
{
  testing::MockFunction<int()> foo_mock1;
  testing::MockFunction<void(int)> foo_mock2;

  EXPECT_CALL(foo_mock2, Call(0)).Times(2);

  auto sch = std::make_shared<translator::AsioSchedule>();
  sch->spawn(std::bind(foo3,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock1.AsStdFunction(),
                       foo_mock2.AsStdFunction()));
  sch->spawn(std::bind(foo3,
                       std::placeholders::_1,
                       std::placeholders::_2,
                       foo_mock1.AsStdFunction(),
                       foo_mock2.AsStdFunction()));

  std::thread th(std::bind(&translator::Schedule::run, sch));

  if (th.joinable())
    th.join();
}
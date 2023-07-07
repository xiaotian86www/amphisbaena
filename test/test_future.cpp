#include <chrono>
#include <memory>
#include <thread>

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
  translator::Promise<int> pms(sch, co);
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
  translator::Promise<int> pms(sch, co);
  func2(pms.future().get_for(1, 0));
}

class Future : public testing::Test
{
public:
  virtual void SetUp() { sch = std::make_shared<translator::Schedule>(ios); }

  virtual void TearDown() {}

protected:
  boost::asio::io_service ios;
  std::thread th;
  std::shared_ptr<translator::Schedule> sch;
};

TEST_F(Future, get)
{
  testing::MockFunction<int()> foo_mock1;
  testing::MockFunction<void(int)> foo_mock2;

  EXPECT_CALL(foo_mock1, Call()).Times(2).WillRepeatedly(testing::Return(1));
  EXPECT_CALL(foo_mock2, Call(1)).Times(2);

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
  
  th = std::thread([this] { ios.run(); });

  if (th.joinable())
    th.join();
}

TEST_F(Future, get_for)
{
  testing::MockFunction<int()> foo_mock1;
  testing::MockFunction<void(int)> foo_mock2;

  EXPECT_CALL(foo_mock1, Call()).Times(2).WillRepeatedly(testing::Return(1));
  EXPECT_CALL(foo_mock2, Call(1)).Times(2);

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

  th = std::thread([this] { ios.run(); });

  if (th.joinable())
    th.join();
}

TEST_F(Future, get_for_timeout)
{
  testing::MockFunction<int()> foo_mock1;
  testing::MockFunction<void(int)> foo_mock2;

  EXPECT_CALL(foo_mock2, Call(0)).Times(2);

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

  th = std::thread([this] { ios.run(); });

  if (th.joinable())
    th.join();
}
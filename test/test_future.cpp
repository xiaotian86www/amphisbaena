#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include "fixture_schedule.hpp"
#include "future.hpp"
#include "schedule.hpp"

class Future : public FixtureSchedule
{
protected:
  testing::MockFunction<int()> foo_mock1;
  testing::MockFunction<void(int)> foo_mock2;
};

TEST_F(Future, get)
{
  auto foo = [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
    amphisbaena::Promise<int> pms(sch, co);
    pms.set(foo_mock1.Call());
    foo_mock2.Call(pms.future().get());
  };

  EXPECT_CALL(foo_mock1, Call()).Times(2).WillRepeatedly(testing::Return(1));
  EXPECT_CALL(foo_mock2, Call(1)).Times(2);

  sch->spawn(foo);
  sch->spawn(foo);
}

TEST_F(Future, get_for)
{
  auto foo = [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
    co.yield_for(0);
    foo_mock2.Call(foo_mock1.Call());
  };

  EXPECT_CALL(foo_mock1, Call()).Times(2).WillRepeatedly(testing::Return(1));
  EXPECT_CALL(foo_mock2, Call(1)).Times(2);

  sch->spawn(foo);
  sch->spawn(foo);
}

TEST_F(Future, get_for_timeout)
{
  auto foo = [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
    amphisbaena::Promise<int> pms(sch, co);
    foo_mock2.Call(pms.future().get_for(1, 10));
  };

  EXPECT_CALL(foo_mock2, Call(10)).Times(2);

  sch->spawn(foo);
  sch->spawn(foo);
}
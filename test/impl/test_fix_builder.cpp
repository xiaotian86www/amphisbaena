#include "gmock/gmock.h"
#include <boost/asio/io_service.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <quickfix/FixValues.h>
#include <thread>

#include "environment.hpp"
#include "impl/fix_builder.hpp"
#include "impl/fix_message.hpp"
#include "mock/mock_service.hpp"
#include "schedule.hpp"
#include "fixture/fixture_schedule.hpp"

class FixBuilder : public FixtureSchedule
{
public:
  FixBuilder()
    : service(new testing::NiceMock<MockService>())
    , builder(std::unique_ptr<translator::Service>(service), 1)
  {
    translator::detail::get_field_info::init(
      "/usr/local/share/quickfix/FIX42.xml");
  }

protected:
  testing::NiceMock<MockService>* service;
  translator::FixMessageBuilder builder;
};

TEST_F(FixBuilder, call)
{
  auto request = std::make_shared<translator::FixMessage>();
  auto& req_head = request->get_head();
  req_head.set_value("MsgType", FIX::MsgType_NewOrderSingle);
  req_head.set_value("BeginString", "FIX.4.2");
  req_head.set_value("SenderCompID", "CLIENT1");
  req_head.set_value("TargetCompID", "EXECUTOR");

  auto& req_body = request->get_body();
  req_body.set_value("ClOrdID", "100001");
  req_body.set_value("HandlInst", "1");
  req_body.set_value("OrdType", "1");
  req_body.set_value("Symbol", "AAPL");
  req_body.set_value("Side", "1");
  req_body.set_value("TransactTime", "20230718-04:57:20.922010000");

  auto response = std::make_shared<translator::FixMessage>();
  auto& rsp_head = response->get_head();
  rsp_head.set_value("MsgType", FIX::MsgType_ExecutionReport);
  rsp_head.set_value("BeginString", "FIX.4.2");
  rsp_head.set_value("SenderCompID", "EXECUTOR");
  rsp_head.set_value("TargetCompID", "CLIENT1");

  auto& rsp_body = response->get_body();
  rsp_body.set_value("ClOrdID", "100001");

  EXPECT_CALL(*service, send(testing::_))
    .WillOnce(testing::Invoke([response, this](translator::MessagePtr) {
      sch->spawn([response, this](translator::ScheduleRef sch,
                                 translator::CoroutineRef co) {
        EXPECT_NO_THROW(service->handler->on_message(response));
      });
    }));

  sch->spawn(
    [request, response, this](translator::ScheduleRef sch_, translator::CoroutineRef co_) {
      translator::Environment env;
      env.sch = sch_;
      env.co = co_;

      auto response_ = builder(env, request);
      EXPECT_EQ(response, response_);
    });
}

TEST_F(FixBuilder, timeout)
{
  auto request = std::make_shared<translator::FixMessage>();
  auto& req_head = request->get_head();
  req_head.set_value("MsgType", FIX::MsgType_NewOrderSingle);
  req_head.set_value("BeginString", "FIX.4.2");
  req_head.set_value("SenderCompID", "CLIENT1");
  req_head.set_value("TargetCompID", "EXECUTOR");

  auto& req_body = request->get_body();
  req_body.set_value("ClOrdID", "100001");
  req_body.set_value("HandlInst", "1");
  req_body.set_value("OrdType", "1");
  req_body.set_value("Symbol", "AAPL");
  req_body.set_value("Side", "1");
  req_body.set_value("TransactTime", "20230718-04:57:20.922010000");

  EXPECT_CALL(*service, send(testing::_)).Times(1);

  sch->spawn(
    [request, this](translator::ScheduleRef sch_, translator::CoroutineRef co_) {
      translator::Environment env;
      env.sch = sch_;
      env.co = co_;

      auto response = builder(env, request);
      EXPECT_EQ(response, nullptr);
    });
}
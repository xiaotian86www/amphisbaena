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

class FixBuilder : public testing::Test
{
public:
  FixBuilder()
    : sch(std::make_shared<translator::Schedule>(ios_))
    , service(new testing::NiceMock<MockService>())
    , builder(std::unique_ptr<translator::Service>(service))
  {
    translator::detail::get_field_info::init(
      "/usr/local/share/quickfix/FIX42.xml");
  }

public:
  virtual void SetUp()
  {
    work_ = std::make_unique<boost::asio::io_service::work>(ios_);
    th_ = std::thread([this] { ios_.run(); });
  }

  virtual void TearDown()
  {
    work_.reset();
    th_.join();
  }

private:
  boost::asio::io_service ios_;
  std::unique_ptr<boost::asio::io_service::work> work_;
  std::thread th_;

protected:
  std::shared_ptr<translator::Schedule> sch;
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
      // EXPECT_NO_THROW(service->handler->on_message(response));
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
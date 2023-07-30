#include "gmock/gmock.h"
#include <boost/asio/io_service.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <quickfix/FixValues.h>
#include <thread>

#include "builder/fix_client/fix_builder.hpp"
#include "environment.hpp"
#include "fixture/fixture_schedule.hpp"
#include "impl/fix_message.hpp"
#include "mock/mock_builder.hpp"
#include "mock/mock_client.hpp"
#include "mock/mock_session.hpp"
#include "schedule.hpp"

class FixBuilder : public FixtureSchedule
{
public:
  FixBuilder()
    : session(std::make_shared<MockSession>())
    , builder(client_factory, 1)
  {
    translator::FixMessage::init("/usr/local/share/quickfix/FIX42.xml");
  }

protected:
  MockClientFactory client_factory;
  std::shared_ptr<MockSession> session;
  translator::FixBuilder builder;
};

TEST_F(FixBuilder, call)
{
  auto request = std::make_shared<translator::FixMessage>();
  auto req_head = request->get_head();
  req_head->set_value("MsgType", FIX::MsgType_NewOrderSingle);
  req_head->set_value("BeginString", "FIX.4.2");
  req_head->set_value("SenderCompID", "CLIENT1");
  req_head->set_value("TargetCompID", "EXECUTOR");

  auto req_body = request->get_body();
  req_body->set_value("ClOrdID", "100001");
  req_body->set_value("HandlInst", "1");
  req_body->set_value("OrdType", "1");
  req_body->set_value("Symbol", "AAPL");
  req_body->set_value("Side", "1");
  req_body->set_value("TransactTime", "20230718-04:57:20.922010000");

  auto response = std::make_shared<translator::FixMessage>();
  auto rsp_head = response->get_head();
  rsp_head->set_value("MsgType", FIX::MsgType_ExecutionReport);
  rsp_head->set_value("BeginString", "FIX.4.2");
  rsp_head->set_value("SenderCompID", "EXECUTOR");
  rsp_head->set_value("TargetCompID", "CLIENT1");

  auto rsp_body = response->get_body();
  rsp_body->set_value("ClOrdID", "100001");

  EXPECT_CALL(*client_factory.client, create(testing::_)).WillOnce(testing::Return(session));

  EXPECT_CALL(*session, send(testing::_))
    .WillOnce(testing::Invoke([this, response](translator::MessagePtr) {
      sch->spawn([this, response](translator::ScheduleRef sch,
                                  translator::CoroutineRef co) {
        EXPECT_NO_THROW(
          client_factory.client->send(translator::ScheduleRef(),
                                            translator::CoroutineRef(),
                                            session,
                                            response));
      });
    }));

  sch->spawn([request, response, this](translator::ScheduleRef sch_,
                                       translator::CoroutineRef co_) {
    translator::Environment env;
    env.sch = sch_;
    env.co = co_;

    auto response_ = builder.create(env, request);
    EXPECT_EQ(response, response_);
  });
}

TEST_F(FixBuilder, timeout)
{
  auto request = std::make_shared<translator::FixMessage>();
  auto req_head = request->get_head();
  req_head->set_value("MsgType", FIX::MsgType_NewOrderSingle);
  req_head->set_value("BeginString", "FIX.4.2");
  req_head->set_value("SenderCompID", "CLIENT1");
  req_head->set_value("TargetCompID", "EXECUTOR");

  auto req_body = request->get_body();
  req_body->set_value("ClOrdID", "100001");
  req_body->set_value("HandlInst", "1");
  req_body->set_value("OrdType", "1");
  req_body->set_value("Symbol", "AAPL");
  req_body->set_value("Side", "1");
  req_body->set_value("TransactTime", "20230718-04:57:20.922010000");

  EXPECT_CALL(*client_factory.client, create(testing::_)).WillOnce(testing::Return(session));

  EXPECT_CALL(*session, send(testing::_)).Times(1);

  sch->spawn([request, this](translator::ScheduleRef sch_,
                             translator::CoroutineRef co_) {
    translator::Environment env;
    env.sch = sch_;
    env.co = co_;

    auto response = builder.create(env, request);
    EXPECT_EQ(response, nullptr);
  });
}
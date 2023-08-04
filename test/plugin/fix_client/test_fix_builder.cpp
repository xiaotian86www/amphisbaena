
#include <boost/asio/io_service.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <quickfix/FixValues.h>
#include <thread>

#include "environment.hpp"
#include "fixture_schedule.hpp"
#include "mock_builder.hpp"
#include "mock_session.hpp"
#include "mock_client.hpp"
#include "plugin/fix_client/fix_builder.hpp"
#include "plugin/fix_client/fix_message.hpp"
#include "schedule.hpp"

class FixBuilder : public FixtureSchedule
{
public:
  FixBuilder()
    : session(std::make_shared<MockSession>())
    , builder(client_factory, 1)
  {
    amphisbaena::FixMessage::init("/usr/local/share/quickfix/FIX42.xml");
  }

protected:
  MockClientFactory client_factory;
  std::shared_ptr<MockSession> session;
  amphisbaena::FixBuilder builder;
};

TEST_F(FixBuilder, call)
{
  auto request = std::make_shared<amphisbaena::FixMessage>();
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

  auto response = std::make_shared<amphisbaena::FixMessage>();
  auto rsp_head = response->get_head();
  rsp_head->set_value("MsgType", FIX::MsgType_ExecutionReport);
  rsp_head->set_value("BeginString", "FIX.4.2");
  rsp_head->set_value("SenderCompID", "EXECUTOR");
  rsp_head->set_value("TargetCompID", "CLIENT1");

  auto rsp_body = response->get_body();
  rsp_body->set_value("ClOrdID", "100001");

  EXPECT_CALL(*client_factory.client, create(testing::_))
    .WillOnce(testing::Return(session));

  EXPECT_CALL(*session, send(testing::_))
    .WillOnce(testing::Invoke([this, response](amphisbaena::MessagePtr) {
      sch->spawn([this, response](amphisbaena::ScheduleRef sch,
                                  amphisbaena::CoroutineRef co) {
        EXPECT_NO_THROW(client_factory.client->send(amphisbaena::ScheduleRef(),
                                                    amphisbaena::CoroutineRef(),
                                                    session,
                                                    response));
      });
    }));

  sch->spawn([request, response, this](amphisbaena::ScheduleRef sch_,
                                       amphisbaena::CoroutineRef co_) {
    amphisbaena::Environment env;
    env.sch = sch_;
    env.co = co_;

    auto response_ = builder.create(env, request);
    EXPECT_EQ(response, response_);
  });
}

TEST_F(FixBuilder, timeout)
{
  auto request = std::make_shared<amphisbaena::FixMessage>();
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

  EXPECT_CALL(*client_factory.client, create(testing::_))
    .WillOnce(testing::Return(session));

  EXPECT_CALL(*session, send(testing::_)).Times(1);

  sch->spawn([request, this](amphisbaena::ScheduleRef sch_,
                             amphisbaena::CoroutineRef co_) {
    amphisbaena::Environment env;
    env.sch = sch_;
    env.co = co_;

    EXPECT_THROW(builder.create(env, request), amphisbaena::TimeoutException);
  });
}

#include <boost/asio/io_service.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-copy"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <quickfix/FixValues.h>
#pragma GCC diagnostic pop

#include "environment.hpp"
#include "fixture_schedule.hpp"
#include "mock_builder.hpp"
#include "mock_client.hpp"
#include "mock_session.hpp"
#include "plugin/fix_client/fix_builder.hpp"
#include "plugin/fix_client/fix_message.hpp"
#include "schedule.hpp"

class FixBuilder : public FixtureSchedule<testing::Test>
{
public:
  FixBuilder()
    : builder(client_factory, 1)
    , session(std::make_shared<MockSession>(amphisbaena::ScheduleRef(),
                                            amphisbaena::CoroutineRef(),
                                            builder))
  {
  }

protected:
  MockClientFactory client_factory;
  amphisbaena::FixBuilder builder;
  std::shared_ptr<MockSession> session;
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
      sch->spawn([this, response](amphisbaena::ScheduleRef /* sch */,
                                  amphisbaena::CoroutineRef /* co */) {
        session->do_recv(response);
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
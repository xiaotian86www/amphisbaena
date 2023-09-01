
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <string_view>

#include "builder.hpp"
#include "common/http_message/http_message.hpp"
#include "environment.hpp"
#include "fixture_schedule.hpp"
#include "matcher_message.hpp"
#include "message.hpp"
#include "mock_builder.hpp"
#include "plugin/fix_client/fix_message.hpp"
#include "plugin/http_to_fix/http_to_fix_builder.hpp"
#include "schedule.hpp"

class HttpToFixBuilder : public FixtureSchedule<testing::Test>
{
public:
  HttpToFixBuilder()
    : fix_builder(std::make_shared<MockMessageBuilder>("fix"))
    , fix_message_factory(std::make_shared<amphisbaena::FixMessageFactory>())
    , http_message_factory(std::make_shared<amphisbaena::HttpMessageFactory>())
  {
    amphisbaena::MessageFactory::registe(fix_message_factory);
    amphisbaena::MessageFactory::registe(http_message_factory);

    amphisbaena::MessageBuilder::registe(fix_builder);
  }

  ~HttpToFixBuilder()
  {
    amphisbaena::MessageBuilder::unregiste();

    amphisbaena::MessageFactory::unregiste();
  }

protected:
  std::shared_ptr<MockMessageBuilder> fix_builder;
  std::shared_ptr<amphisbaena::MessageFactory> fix_message_factory;
  std::shared_ptr<amphisbaena::MessageFactory> http_message_factory;
  amphisbaena::builder::HttpToFixBuilder http_to_fix_builder;
};

TEST_F(HttpToFixBuilder, call)
{
  sch->spawn(
    [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
      amphisbaena::Environment env;
      env.sch = sch;
      env.co = co;

      EXPECT_CALL(*fix_builder,
                  create(testing::Ref(env),
                         testing::Truly([](amphisbaena::MessagePtr request) {
                           auto request_head = request->get_head();
                           auto request_body = request->get_body();
                           return request_head->get_string("SenderCompID") ==
                                    "CLIENT1" &&
                                  request_head->get_int("MsgSeqNum") == 1 &&
                                  request_body->get_double("LeavesQty") == 1.01;
                         })))
        .WillOnce(testing::Invoke([] {
          auto response = amphisbaena::MessageFactory::create("fix");
          auto request_head = response->get_head();
          auto response_body = response->get_body();
          request_head->set_value("SenderCompID", "CLIENT2");
          request_head->set_value("MsgSeqNum", 2);
          response_body->set_value("LeavesQty", 2.01);
          return response;
        }));

      auto request = amphisbaena::MessageFactory::create("Http");

      auto request_body = request->get_body();
      auto request_body_head = request_body->get_or_set_object("head");
      auto request_body_body = request_body->get_or_set_object("body");
      request_body_head->set_value("SenderCompID", "CLIENT1");
      request_body_head->set_value("MsgSeqNum", 1);
      request_body_body->set_value("LeavesQty", 1.01);

      auto response = http_to_fix_builder.create(env, request);

      auto response_body = response->get_body();
      auto response_body_head = response_body->get_or_set_object("head");
      auto response_body_body = response_body->get_or_set_object("body");

      EXPECT_THAT(response_body_head,
                  testing::AllOf(field_string_eq("SenderCompID", "CLIENT2"),
                                 field_int_eq("MsgSeqNum", 2)));

      EXPECT_THAT(response_body_body,
                  testing::AllOf(field_double_eq("LeavesQty", 2.01)));
    });
}
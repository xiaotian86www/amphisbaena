
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <string_view>

#include "builder.hpp"
#include "environment.hpp"
#include "fixture/fixture_schedule.hpp"
#include "impl/json_message.hpp"
#include "matcher/matcher_message.hpp"
#include "message.hpp"
#include "mock/mock_builder.hpp"
#include "plugin/http_to_fix/http_to_fix_builder.hpp"
#include "schedule.hpp"

class HttpToFixBuilder : public FixtureSchedule
{
public:
  HttpToFixBuilder()
    : fix_builder(std::make_shared<MockMessageBuilder>("fix"))
  {
    amphisbaena::MessageFactory::registe(
      "Fix", [] { return std::make_shared<amphisbaena::JsonMessage>(); });
    amphisbaena::MessageFactory::registe(
      "Json", [] { return std::make_shared<amphisbaena::JsonMessage>(); });

    amphisbaena::MessageBuilder::registe(fix_builder->name(), fix_builder);
  }

  ~HttpToFixBuilder()
  {
    amphisbaena::MessageBuilder::unregiste();

    amphisbaena::MessageFactory::unregiste();
  }

protected:
  amphisbaena::builder::HttpToFixBuilder http_to_fix_builder;
  std::shared_ptr<MockMessageBuilder> fix_builder;
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
                           auto request_body = request->get_body();
                           return request_body->get_string("SenderCompID") ==
                                    "CLIENT1" &&
                                  request_body->get_int("MsgSeqNum") == 1 &&
                                  request_body->get_double("LeavesQty") == 1.01;
                         })))
        .WillOnce(testing::Invoke([] {
          auto response = amphisbaena::MessageFactory::create("Fix");

          auto response_body = response->get_body();
          response_body->set_value("SenderCompID", "CLIENT2");
          response_body->set_value("MsgSeqNum", 2);
          response_body->set_value("LeavesQty", 2.01);
          return response;
        }));

      auto request = amphisbaena::MessageFactory::create("Json");

      auto request_body = request->get_body();
      request_body->set_value("SenderCompID", "CLIENT1");
      request_body->set_value("MsgSeqNum", 1);
      request_body->set_value("LeavesQty", 1.01);

      auto response = http_to_fix_builder.create(env, request);

      auto response_body = response->get_body();

      EXPECT_THAT(response_body,
                  testing::AllOf(field_string_eq("SenderCompID", "CLIENT2"),
                                 field_int_eq("MsgSeqNum", 2),
                                 field_double_eq("LeavesQty", 2.01)));
    });
}
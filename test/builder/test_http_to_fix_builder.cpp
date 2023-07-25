
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "builder.hpp"
#include "builder/http_to_fix/builder.hpp"
#include "environment.hpp"
#include "fixture/fixture_schedule.hpp"
#include "impl/fix_message.hpp"
#include "matcher/matcher_message.hpp"
#include "message.hpp"
#include "schedule.hpp"

class HttpToFixBuilder : public FixtureSchedule
{
public:
  HttpToFixBuilder()
    : builder(std::make_shared<translator::MessageBuilder>())
  {
    translator::FixMessage::init("/usr/local/share/quickfix/FIX42.xml");
  }

protected:
  translator::MessageBuilderPtr builder;
  translator::builder::HttpToFixBuilder http_to_fix_builder;
};

TEST_F(HttpToFixBuilder, call)
{
  sch->spawn([this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    translator::Environment env;
    env.builder = builder;
    env.sch = sch;
    env.co = co;

    testing::MockFunction<translator::MessagePtr(translator::Environment&,
                                                 translator::MessagePtr)>
      fix_builder;

    EXPECT_CALL(fix_builder,
                Call(testing::Ref(env),
                     testing::Truly([](translator::MessagePtr request) {
                       auto request_body = request->get_body();
                       return request_body->get_string("SenderCompID") ==
                                "CLIENT1" &&
                              request_body->get_int("MsgSeqNum") == 1 &&
                              request_body->get_double("LeavesQty") == 1.01;
                     })))
      .WillOnce(testing::Invoke([] {
        auto response =
          translator::MessageFactory::create(translator::MessageType::kFix);

        auto response_body = response->get_body();
        response_body->set_value("SenderCompID", "CLIENT2");
        response_body->set_value("MsgSeqNum", 2);
        response_body->set_value("LeavesQty", 2.01);
        return response;
      }));

    builder->registe("fix", fix_builder.AsStdFunction());

    auto request =
      translator::MessageFactory::create(translator::MessageType::kJson);

    auto request_body = request->get_body();
    request_body->set_value("SenderCompID", "CLIENT1");
    request_body->set_value("MsgSeqNum", 1);
    request_body->set_value("LeavesQty", 1.01);

    auto response = http_to_fix_builder(env, request);

    auto response_body = response->get_body();

    EXPECT_THAT(response_body,
                testing::AllOf(field_string_eq("SenderCompID", "CLIENT2"),
                               field_int_eq("MsgSeqNum", 2),
                               field_double_eq("LeavesQty", 2.01)));
  });
}
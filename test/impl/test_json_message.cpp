
#include "fixture/fixture_message.hpp"
#include "impl/json_message.hpp"

translator::MessagePtr
create_json_message()
{
  return std::make_shared<translator::JsonMessage>();
}

INSTANTIATE_TEST_SUITE_P(Json, Message, testing::Values(create_json_message));

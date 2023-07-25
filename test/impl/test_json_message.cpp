
#include "fixture/fixture_message.hpp"
#include "impl/json_message.hpp"

static auto ctor_func = [] {
  return std::make_shared<translator::JsonMessage>();
};

INSTANTIATE_TEST_SUITE_P(Json, Message, testing::Values(ctor_func));

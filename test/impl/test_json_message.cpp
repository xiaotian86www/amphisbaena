
#include "fixture/fixture_message.hpp"
#include "impl/json_message.hpp"
#include "message.hpp"

static std::shared_ptr<amphisbaena::MessageFactory> factory(
  std::make_shared<amphisbaena::JsonMessageFactory>());

INSTANTIATE_TEST_SUITE_P(Json, Message, testing::Values(factory));

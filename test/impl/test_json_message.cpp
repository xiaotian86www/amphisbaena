
#include "fixture/fixture_message.hpp"
#include "message.hpp"

INSTANTIATE_TEST_SUITE_P(Json,
                         Message,
                         testing::Values(translator::MessageType::kJson));

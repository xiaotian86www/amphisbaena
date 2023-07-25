
#include "fixture/fixture_message.hpp"
#include "message.hpp"

// TODO FixMessage没有加载元数据

INSTANTIATE_TEST_SUITE_P(Fix,
                         Message,
                         testing::Values(translator::MessageType::kFix));

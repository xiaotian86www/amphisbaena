
#include "fixture/fixture_message.hpp"
#include "plugin/fix_client/fix_message.hpp"

// TODO FixMessage没有加载元数据

static std::shared_ptr<amphisbaena::MessageFactory> factory(
  std::make_shared<amphisbaena::FixMessageFactory>());

INSTANTIATE_TEST_SUITE_P(Fix, Message, testing::Values(factory));

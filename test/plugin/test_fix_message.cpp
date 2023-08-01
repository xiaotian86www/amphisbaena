
#include "fixture/fixture_message.hpp"
#include "plugin/fix_client/fix_message.hpp"

// TODO FixMessage没有加载元数据

static auto ctor_func = [] {
  return std::make_shared<amphisbaena::FixMessage>();
};

INSTANTIATE_TEST_SUITE_P(Fix, Message, testing::Values(ctor_func));

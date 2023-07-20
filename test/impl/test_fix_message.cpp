
#include "fixture/fixture_message.hpp"
#include "impl/fix_message.hpp"

translator::MessagePtr
create_fix_message()
{
  translator::detail::get_field_info::init(
    "/usr/local/share/quickfix/FIX42.xml");
  return std::make_shared<translator::FixMessage>();
}

INSTANTIATE_TEST_SUITE_P(Fix, Message, testing::Values(create_fix_message));

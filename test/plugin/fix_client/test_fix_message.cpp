
#include "plugin/fix_client/fix_message.hpp"
#include "test_message.ipp"

INSTANTIATE_TYPED_TEST_SUITE_P(Fix, Message, amphisbaena::FixMessageFactory);

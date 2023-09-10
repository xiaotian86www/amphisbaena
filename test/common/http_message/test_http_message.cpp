
#include "common/http_message/http_message.hpp"
#include "test_message.ipp"
#include "message.hpp"

INSTANTIATE_TYPED_TEST_SUITE_P(Http, Message, amphisbaena::HttpMessageFactory);
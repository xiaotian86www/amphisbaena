#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>

#include "message.hpp"

using ctor_prototype = translator::MessagePtr();

class Message : public testing::TestWithParam<std::function<ctor_prototype>>
{
public:
  void SetUp() { message = GetParam()(); }

  void TearDown() {}

protected:
  translator::MessagePtr message;
};

TEST_P(Message, get_int)
{
  auto& body = message->get_body();
  body.set_value("MsgSeqNum", 1);

  EXPECT_EQ(body.get_int("MsgSeqNum"), 1);
  EXPECT_EQ(body.get_value("MsgSeqNum", 0), 1);

  EXPECT_THROW(body.get_int("SenderCompID"), translator::NoKeyException);
  EXPECT_EQ(body.get_value("SenderCompID", 10), 10);

  body.set_value("SenderCompID", "a");

  EXPECT_THROW(body.get_int("SenderCompID"), translator::TypeExecption);
  EXPECT_EQ(body.get_value("SenderCompID", 10), 10);
}

TEST_P(Message, get_string)
{
  auto& body = message->get_body();
  body.set_value("SenderCompID", "value1");
  EXPECT_EQ(body.get_string("SenderCompID"), "value1");
  EXPECT_EQ(body.get_value("SenderCompID", ""), "value1");

  EXPECT_THROW(body.get_string("MsgSeqNum"), translator::NoKeyException);
  EXPECT_EQ(body.get_value("MsgSeqNum", ""), "");

  body.set_value("MsgSeqNum", 1);

  EXPECT_THROW(body.get_string("MsgSeqNum"), translator::TypeExecption);
  EXPECT_EQ(body.get_value("MsgSeqNum", ""), "");
}

#include "detail/json_message.hpp"

translator::MessagePtr
create_json_message()
{
  return std::make_unique<translator::JsonMessage>();
}

INSTANTIATE_TEST_SUITE_P(Json, Message, testing::Values(create_json_message));

#include "detail/fix_message.hpp"

struct create_fix_message
{
  create_fix_message()
    : dd("/usr/local/share/quickfix/FIX42.xml")
  {
  }

  translator::MessagePtr operator()() const
  {
    return std::make_unique<translator::FixMessage>(dd);
  }

  FIX::DataDictionary dd;
};

INSTANTIATE_TEST_SUITE_P(Fix, Message, testing::Values(create_fix_message()));

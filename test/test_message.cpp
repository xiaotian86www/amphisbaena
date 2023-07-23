#include "fixture/fixture_message.hpp"

TEST_P(Message, get_int)
{
  auto body = message->get_body();
  body->set_value("MsgSeqNum", 1);

  EXPECT_EQ(body->get_int("MsgSeqNum"), 1);
  EXPECT_EQ(body->get_value("MsgSeqNum", 0), 1);

  EXPECT_THROW(body->get_int("SenderCompID"), translator::NoKeyException);
  EXPECT_EQ(body->get_value("SenderCompID", 10), 10);

  body->set_value("SenderCompID", "a");

  EXPECT_THROW(body->get_int("SenderCompID"), translator::TypeExecption);
  EXPECT_EQ(body->get_value("SenderCompID", 10), 10);
}

TEST_P(Message, get_string)
{
  auto body = message->get_body();
  body->set_value("SenderCompID", "value1");
  EXPECT_EQ(body->get_string("SenderCompID"), "value1");
  EXPECT_EQ(body->get_value("SenderCompID", ""), "value1");

  EXPECT_THROW(body->get_string("MsgSeqNum"), translator::NoKeyException);
  EXPECT_EQ(body->get_value("MsgSeqNum", ""), "");

  body->set_value("MsgSeqNum", 1);

  EXPECT_THROW(body->get_string("MsgSeqNum"), translator::TypeExecption);
  EXPECT_EQ(body->get_value("MsgSeqNum", ""), "");
}

TEST_P(Message, get_string_char)
{
  auto body = message->get_body();
  body->set_value("OrdType", "1");
  EXPECT_EQ(body->get_string("OrdType"), "1");
  EXPECT_EQ(body->get_value("OrdType", ""), "1");

  EXPECT_THROW(body->get_string("MsgSeqNum"), translator::NoKeyException);
  EXPECT_EQ(body->get_value("MsgSeqNum", ""), "");

  body->set_value("MsgSeqNum", 1);

  EXPECT_THROW(body->get_string("MsgSeqNum"), translator::TypeExecption);
  EXPECT_EQ(body->get_value("MsgSeqNum", ""), "");
}

TEST_P(Message, get_string_timestamp)
{
  auto body = message->get_body();
  body->set_value("TransactTime", "20230718-04:57:20.922010000");
  EXPECT_EQ(body->get_string("TransactTime"), "20230718-04:57:20.922010000");
  EXPECT_EQ(body->get_value("TransactTime", ""), "20230718-04:57:20.922010000");

  EXPECT_THROW(body->get_string("MsgSeqNum"), translator::NoKeyException);
  EXPECT_EQ(body->get_value("MsgSeqNum", ""), "");

  body->set_value("MsgSeqNum", 1);

  EXPECT_THROW(body->get_string("MsgSeqNum"), translator::TypeExecption);
  EXPECT_EQ(body->get_value("MsgSeqNum", ""), "");
}

TEST_P(Message, get_double)
{
  auto body = message->get_body();
  body->set_value("LeavesQty", 1.01);

  EXPECT_EQ(body->get_double("LeavesQty"), 1.01);
  EXPECT_EQ(body->get_value("LeavesQty", 0.01), 1.01);

  EXPECT_THROW(body->get_double("SenderCompID"), translator::NoKeyException);
  EXPECT_EQ(body->get_value("SenderCompID", 0.01), 0.01);

  body->set_value("SenderCompID", "a");

  EXPECT_THROW(body->get_double("SenderCompID"), translator::TypeExecption);
  EXPECT_EQ(body->get_value("SenderCompID", 0.01), 0.01);
}

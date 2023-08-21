#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fixture_message.hpp"
#include "matcher_message.hpp"
#include "message.hpp"

TEST_P(Message, get_int)
{
  auto message = factory->create();
  auto body = message->get_body();
  body->set_value("MsgSeqNum", 1);

  EXPECT_EQ(body->get_int("MsgSeqNum"), 1);
  EXPECT_EQ(body->get_value("MsgSeqNum", 0), 1);

  EXPECT_THROW(body->get_int("SenderCompID"), amphisbaena::NoKeyException);
  EXPECT_EQ(body->get_value("SenderCompID", 10), 10);

  body->set_value("SenderCompID", "a");

  EXPECT_THROW(body->get_int("SenderCompID"), amphisbaena::TypeExecption);
  EXPECT_EQ(body->get_value("SenderCompID", 10), 10);
}

TEST_P(Message, get_string)
{
  auto message = factory->create();
  auto body = message->get_body();
  body->set_value("SenderCompID", "value1");
  EXPECT_EQ(body->get_string("SenderCompID"), "value1");
  EXPECT_EQ(body->get_value("SenderCompID", ""), "value1");

  EXPECT_THROW(body->get_string("MsgSeqNum"), amphisbaena::NoKeyException);
  EXPECT_EQ(body->get_value("MsgSeqNum", ""), "");

  body->set_value("MsgSeqNum", 1);

  EXPECT_THROW(body->get_string("MsgSeqNum"), amphisbaena::TypeExecption);
  EXPECT_EQ(body->get_value("MsgSeqNum", ""), "");
}

TEST_P(Message, get_string_char)
{
  auto message = factory->create();
  auto body = message->get_body();
  body->set_value("OrdType", "1");
  EXPECT_EQ(body->get_string("OrdType"), "1");
  EXPECT_EQ(body->get_value("OrdType", ""), "1");

  EXPECT_THROW(body->get_string("MsgSeqNum"), amphisbaena::NoKeyException);
  EXPECT_EQ(body->get_value("MsgSeqNum", ""), "");

  body->set_value("MsgSeqNum", 1);

  EXPECT_THROW(body->get_string("MsgSeqNum"), amphisbaena::TypeExecption);
  EXPECT_EQ(body->get_value("MsgSeqNum", ""), "");
}

TEST_P(Message, get_string_timestamp)
{
  auto message = factory->create();
  auto body = message->get_body();
  body->set_value("TransactTime", "20230718-04:57:20.922010000");
  EXPECT_EQ(body->get_string("TransactTime"), "20230718-04:57:20.922010000");
  EXPECT_EQ(body->get_value("TransactTime", ""), "20230718-04:57:20.922010000");

  EXPECT_THROW(body->get_string("MsgSeqNum"), amphisbaena::NoKeyException);
  EXPECT_EQ(body->get_value("MsgSeqNum", ""), "");

  body->set_value("MsgSeqNum", 1);

  EXPECT_THROW(body->get_string("MsgSeqNum"), amphisbaena::TypeExecption);
  EXPECT_EQ(body->get_value("MsgSeqNum", ""), "");
}

TEST_P(Message, get_double)
{
  auto message = factory->create();
  auto body = message->get_body();
  body->set_value("LeavesQty", 1.01);

  EXPECT_EQ(body->get_double("LeavesQty"), 1.01);
  EXPECT_EQ(body->get_value("LeavesQty", 0.01), 1.01);

  EXPECT_THROW(body->get_double("SenderCompID"), amphisbaena::NoKeyException);
  EXPECT_EQ(body->get_value("SenderCompID", 0.01), 0.01);

  body->set_value("SenderCompID", "a");

  EXPECT_THROW(body->get_double("SenderCompID"), amphisbaena::TypeExecption);
  EXPECT_EQ(body->get_value("SenderCompID", 0.01), 0.01);
}

TEST_P(Message, iterator)
{
  auto message = factory->create();
  auto body = message->get_body();
  body->set_value("SenderCompID", "CLIENT1");
  body->set_value("MsgSeqNum", 1);
  body->set_value("LeavesQty", 1.01);

  testing::MockFunction<void(amphisbaena::Object::ConstIteratorWrap&)> handler;
  EXPECT_CALL(handler, Call(field_iter_string_eq("SenderCompID", "CLIENT1")));
  EXPECT_CALL(handler, Call(field_iter_int_eq("MsgSeqNum", 1)));
  EXPECT_CALL(handler, Call(field_iter_double_eq("LeavesQty", 1.01)));

  for (auto iter = body->begin(); iter != body->end(); ++iter) {
    handler.Call(iter);
  }
}

TEST_P(Message, assignment)
{
  auto message = factory->create();
  auto message2 = factory->create();
  auto body = message->get_body();
  body->set_value("SenderCompID", "CLIENT1");
  body->set_value("MsgSeqNum", 1);
  body->set_value("LeavesQty", 1.01);
  auto body2 = message2->get_body();
  body2->copy_from(message->get_body());

  EXPECT_THAT(body2, field_string_eq("SenderCompID", "CLIENT1"));
  EXPECT_THAT(body2, field_int_eq("MsgSeqNum", 1));
  EXPECT_THAT(body2, field_double_eq("LeavesQty", 1.01));
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(Message);

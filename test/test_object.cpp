#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>

#include "object.hpp"

using ctor_prototype = translator::ObjectPtr();

class Object : public testing::TestWithParam<std::function<ctor_prototype>>
{
public:
  void SetUp() { obj = GetParam()(); }

  void TearDown() {}

protected:
  translator::ObjectPtr obj;
};

TEST_P(Object, get_int)
{
  auto& root = obj->get_root();
  root.set_value("MsgSeqNum", 1);

  EXPECT_EQ(root.get_int("MsgSeqNum"), 1);
  EXPECT_EQ(root.get_value("MsgSeqNum", 0), 1);

  EXPECT_THROW(root.get_int("SenderCompID"), translator::NoKeyException);
  EXPECT_EQ(root.get_value("SenderCompID", 10), 10);

  root.set_value("SenderCompID", "a");

  EXPECT_THROW(root.get_int("SenderCompID"), translator::TypeExecption);
  EXPECT_EQ(root.get_value("SenderCompID", 10), 10);
}

TEST_P(Object, get_string)
{
  auto& root = obj->get_root();
  root.set_value("SenderCompID", "value1");
  EXPECT_EQ(root.get_string("SenderCompID"), "value1");
  EXPECT_EQ(root.get_value("SenderCompID", ""), "value1");

  EXPECT_THROW(root.get_string("MsgSeqNum"), translator::NoKeyException);
  EXPECT_EQ(root.get_value("MsgSeqNum", ""), "");

  root.set_value("MsgSeqNum", 1);

  EXPECT_THROW(root.get_string("MsgSeqNum"), translator::TypeExecption);
  EXPECT_EQ(root.get_value("MsgSeqNum", ""), "");
}

#include "detail/json_object.hpp"

translator::ObjectPtr
create_json_object()
{
  return std::make_unique<translator::JsonObject>();
}

#include "detail/fix_object.hpp"

struct create_fix_object
{
  create_fix_object()
    : dd("/usr/local/share/quickfix/FIX42.xml")
  {
  }

  translator::ObjectPtr operator()() const
  {
    return std::make_unique<translator::FixObject>(dd);
  }

  FIX::DataDictionary dd;
};

INSTANTIATE_TEST_SUITE_P(JsonFix,
                         Object,
                         testing::Values(create_json_object,
                                         create_fix_object()));

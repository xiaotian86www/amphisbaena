#pragma once

#include <gtest/gtest.h>

#include "message.hpp"

using ctor_prototype = translator::MessagePtr();

class Message : public testing::TestWithParam<translator::MessageType>
{
public:
  void SetUp() { message_type = GetParam(); }

  void TearDown() {}

protected:
  translator::MessageType message_type;
};

#pragma once

#include <gtest/gtest.h>

#include "message.hpp"

using ctor_prototype = translator::MessagePtr();

class Message : public testing::TestWithParam<translator::MessageFactory::ctor_function>
{
public:
  void SetUp() { ctor_func = GetParam(); }

  void TearDown() {}

protected:
  translator::MessageFactory::ctor_function ctor_func;
};

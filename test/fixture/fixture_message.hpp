#pragma once

#include <gtest/gtest.h>

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

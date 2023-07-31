#pragma once

#include <gtest/gtest.h>

#include "message.hpp"

using ctor_prototype = amphisbaena::MessagePtr();

class Message : public testing::TestWithParam<amphisbaena::MessageFactory::ctor_function>
{
public:
  void SetUp() { ctor_func = GetParam(); }

  void TearDown() {}

protected:
  amphisbaena::MessageFactory::ctor_function ctor_func;
};

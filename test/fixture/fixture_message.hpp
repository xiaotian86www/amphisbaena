#pragma once

#include <gtest/gtest.h>

#include "message.hpp"

using ctor_prototype = amphisbaena::MessagePtr();

class Message
  : public testing::TestWithParam<std::shared_ptr<amphisbaena::MessageFactory>>
{
public:
  void SetUp() { factory = GetParam(); }

  void TearDown() {}

protected:
  std::shared_ptr<amphisbaena::MessageFactory> factory;
};

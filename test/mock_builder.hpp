#pragma once

#include <gmock/gmock.h>

#include "builder.hpp"
#include "environment.hpp"
#include "message.hpp"

class MockMessageBuilder : public amphisbaena::MessageBuilder
{
public:
  MockMessageBuilder(std::string name)
    : name_(name)
  {
  }

public:
  MOCK_METHOD(amphisbaena::MessagePtr,
              create,
              (amphisbaena::Environment&, amphisbaena::MessagePtr),
              (override));

  std::string_view name() const override { return name_; }

private:
  std::string name_;
};
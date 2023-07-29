#pragma once

#include <gmock/gmock.h>

#include "builder.hpp"
#include "environment.hpp"
#include "message.hpp"

class MockMessageBuilder : public translator::MessageBuilder
{
public:
  MockMessageBuilder(std::string name)
    : name_(name)
  {
  }

public:
  MOCK_METHOD(translator::MessagePtr,
              create,
              (translator::Environment&, translator::MessagePtr),
              (override));

  std::string_view name() const override { return name_; }

private:
  std::string name_;
};
#pragma once

#include <gmock/gmock.h>

#include "service.hpp"

class MockService : public translator::Service
{
public:
  MOCK_METHOD(void, start, (), (override));

  MOCK_METHOD(void, stop, (), (override));

  MOCK_METHOD(void, send, (translator::MessagePtr message), (override));
};

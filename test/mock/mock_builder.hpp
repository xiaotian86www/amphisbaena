#pragma once

#include <gmock/gmock.h>

#include "message.hpp"
#include "service.hpp"

class MockSession : public translator::Session
{
public:
  MOCK_METHOD(void,
              send,
              (translator::MessagePtr data),
              (override));
};

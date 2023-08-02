#pragma once

#include <gmock/gmock.h>

#include "message.hpp"
#include "session.hpp"

class MockSession : public amphisbaena::Session
{
public:
  MOCK_METHOD(void,
              send,
              (amphisbaena::MessagePtr data),
              (override));
};

#pragma once

#include "gmock/gmock.h"

#include "schedule.hpp"
#include "server.hpp"

class MockServer : public translator::Protocol
{
public:
  MOCK_METHOD(void,
              on_data,
              (std::shared_ptr<translator::Socket>,
               std::shared_ptr<translator::Coroutine>,
               std::string_view),
              (override));
};
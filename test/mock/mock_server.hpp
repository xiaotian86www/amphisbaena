#pragma once

#include "gmock/gmock.h"

#include "schedule.hpp"
#include "server.hpp"

class MockProtocol : public translator::Protocol
{
public:
  MOCK_METHOD(void,
              on_data,
              (std::shared_ptr<translator::Socket>,
               translator::Coroutine,
               std::string_view),
              (override));
};

class MockProtocolFactory : public translator::ProtocolFactory
{
public:
  MOCK_METHOD(std::unique_ptr<translator::Protocol>,
              create,
              (),
              (override));
};
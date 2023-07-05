#pragma once

#include "gmock/gmock.h"

#include "schedule.hpp"
#include "server.hpp"

class MockProtocol : public translator::Protocol
{
public:
  MOCK_METHOD(void,
              on_data,
              (translator::ScheduleRef,
               translator::CoroutineRef,
               std::shared_ptr<translator::Socket>,
               std::string_view),
              (override));
};

class MockProtocolFactory : public translator::ProtocolFactory
{
public:
  MOCK_METHOD(std::unique_ptr<translator::Protocol>, create, (), (override));
};
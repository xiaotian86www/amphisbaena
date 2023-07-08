#pragma once

#include "gmock/gmock.h"

#include "protocol.hpp"
#include "schedule.hpp"

class MockConnection : public translator::Connection
{
public:
  MOCK_METHOD(void,
              send,
              (translator::ScheduleRef,
               translator::CoroutineRef,
               std::string_view),
              (override));
};

class MockProtocol : public translator::Protocol
{
public:
  MOCK_METHOD(void,
              on_data,
              (translator::ScheduleRef,
               translator::CoroutineRef,
               std::shared_ptr<translator::Connection>,
               std::string_view),
              (override));
};

class MockProtocolFactory : public translator::ProtocolFactory
{
public:
  MOCK_METHOD(std::unique_ptr<translator::Protocol>, create, (), (override));
};
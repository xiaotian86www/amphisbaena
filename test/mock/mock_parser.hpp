#pragma once

#include "gmock/gmock.h"

#include "parser.hpp"
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

class MockParser : public translator::Parser
{
public:
  MOCK_METHOD(void,
              on_data,
              (translator::ScheduleRef,
               translator::CoroutineRef,
               translator::ConnectionRef,
               std::string_view),
              (override));
};

class MockParserFactory : public translator::ParserFactory
{
public:
  MOCK_METHOD(std::shared_ptr<translator::Parser>, create, (), (override));
};
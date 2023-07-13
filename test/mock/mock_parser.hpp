#pragma once

#include "gmock/gmock.h"

#include "parser.hpp"
#include "schedule.hpp"

class MockConnection : public translator::Connection
{
public:
  using translator::Connection::Connection;
  MOCK_METHOD(void,
              send,
              (std::string_view),
              (override));
  MOCK_METHOD(std::size_t,
              recv,
              (char* buffer, std::size_t buf_len),
              (override));
  MOCK_METHOD(void,
              close,
              (),
              (override));
};

class MockParser : public translator::Parser
{
public:
  using translator::Parser::Parser;
  MOCK_METHOD(void, on_data, (std::string_view), (override));
};

class MockParserFactory : public translator::ParserFactory
{
public:
  MOCK_METHOD(std::shared_ptr<translator::Parser>,
              create,
              (translator::ScheduleRef,
               translator::CoroutineRef,
               translator::ConnectionRef),
              (override));
};
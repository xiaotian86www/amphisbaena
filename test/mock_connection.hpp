#pragma once

#include <gmock/gmock.h>

#include "message.hpp"
#include "connection.hpp"

class MockConnection : public amphisbaena::Connection
{
public:
  class MockMessageHandler : public MessageHandler
  {
  public:
    MOCK_METHOD(void,
                on_recv,
                (amphisbaena::ScheduleRef sch,
                 amphisbaena::CoroutineRef co,
                 amphisbaena::ConnectionPtr conn,
                 std::string_view data),
                (override));
  };

public:
  using amphisbaena::Connection::Connection;
  MOCK_METHOD(void, send, (std::string_view), (override));
  MOCK_METHOD(bool, recv, (), (override));
  MOCK_METHOD(void, close, (), (override));
};

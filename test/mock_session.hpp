#pragma once

#include <gmock/gmock.h>

#include "message.hpp"
#include "session.hpp"

class MockSession : public amphisbaena::Session
{
public:
  class MockMessageHandler : public MessageHandler
  {
  public:
    MOCK_METHOD(void,
                on_recv,
                (amphisbaena::ScheduleRef sch,
                 amphisbaena::CoroutineRef co,
                 amphisbaena::SessionPtr session,
                 amphisbaena::MessagePtr message),
                (override));
  };

public:
  using amphisbaena::Session::Session;
  MOCK_METHOD(void, send, (amphisbaena::MessagePtr data), (override));
};

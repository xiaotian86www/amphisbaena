#pragma once

#include <gmock/gmock.h>

#include "message.hpp"
#include "session.hpp"

class MockSession : public amphisbaena::Session
{
public:
  MOCK_METHOD(void, send, (amphisbaena::MessagePtr data), (override));

public:
  class MessageHandler : public amphisbaena::Session::MessageHandler
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
};

#pragma once

#include <gmock/gmock.h>

#include "service.hpp"

class MockService : public translator::Service
{
public:
  class MockMessageHandler : public MessageHandler
  {
  public:
    MOCK_METHOD(void,
                on_recv,
                (translator::ScheduleRef sch,
                 translator::CoroutineRef co,
                 translator::SessionPtr session,
                 translator::MessagePtr message),
                (override));
  };

public:
  MOCK_METHOD(void, start, (), (override));

  MOCK_METHOD(void, stop, (), (override));

  MOCK_METHOD(translator::SessionPtr, create, (translator::MessagePtr message), (override));
};

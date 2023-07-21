#include "gmock/gmock.h"
#include <gmock/gmock.h>

#include "server.hpp"

class MockServer : public translator::Server
{
public:
  class MockMessageHandler : public MessageHandler
  {
  public:
    MOCK_METHOD(void,
                on_recv,
                (translator::ScheduleRef sch,
                 translator::CoroutineRef co,
                 translator::ConnectionRef conn,
                 std::string_view data,
                 void** context),
                (override));
  };

public:
  MOCK_METHOD(void, start, (), (override));
  MOCK_METHOD(void, stop, (), (override));
};
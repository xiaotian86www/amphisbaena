#include "gmock/gmock.h"
#include <gmock/gmock.h>

#include "server.hpp"

class MockConnection : public translator::Connection
{
public:
  using translator::Connection::Connection;
  MOCK_METHOD(void,
              send,
              (std::string_view),
              (override));
  // MOCK_METHOD(std::size_t,
  //             recv,
  //             (char* buffer, std::size_t buf_len),
  //             (override));
  MOCK_METHOD(void,
              close,
              (),
              (override));
};

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
                 translator::ConnectionPtr conn,
                 std::string_view data),
                (override));
  };

public:
  MOCK_METHOD(void, start, (), (override));
  MOCK_METHOD(void, stop, (), (override));
};
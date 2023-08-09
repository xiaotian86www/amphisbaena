
#include <gmock/gmock.h>
#include <memory>

#include "server.hpp"

class MockConnection : public amphisbaena::Connection
{
public:
  using amphisbaena::Connection::Connection;
  MOCK_METHOD(void, send, (std::string_view), (override));
  MOCK_METHOD(void, close, (), (override));
};

class MockServer : public amphisbaena::Server
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
  using amphisbaena::Server::Server;
};

class MockServerFactory : public amphisbaena::ServerFactory
{
public:
  MockServerFactory()
    : server(nullptr)
  {
  }

public:
  std::unique_ptr<amphisbaena::Server> create(
    amphisbaena::Server::MessageHandler* handler) override
  {
    server = new MockServer(handler);
    return std::unique_ptr<amphisbaena::Server>(server);
  }

public:
  MockServer* server;
};
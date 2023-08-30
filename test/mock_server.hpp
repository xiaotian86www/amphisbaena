
#include <gmock/gmock.h>
#include <memory>

#include "server.hpp"

class MockServer : public amphisbaena::Server
{
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
    amphisbaena::Connection::MessageHandler& handler) override
  {
    server = new MockServer(handler);
    return std::unique_ptr<amphisbaena::Server>(server);
  }

public:
  MockServer* server;
};
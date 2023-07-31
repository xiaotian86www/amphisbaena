#pragma once

#include <gmock/gmock.h>
#include <memory>

#include "client.hpp"

class MockClient : public amphisbaena::Client
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
  using amphisbaena::Client::Client;

public:
  MOCK_METHOD(amphisbaena::SessionPtr,
              create,
              (amphisbaena::MessagePtr message),
              (override));

  void send(amphisbaena::ScheduleRef sch,
            amphisbaena::CoroutineRef co,
            amphisbaena::SessionPtr session,
            amphisbaena::MessagePtr message)
  {
    message_handler_->on_recv(sch, co, session, message);
  }
};

class MockClientFactory : public amphisbaena::ClientFactory
{
public:
  std::unique_ptr<amphisbaena::Client> create(
    amphisbaena::Client::MessageHandler* handler) override
  {
    client = new MockClient(handler);
    return std::unique_ptr<MockClient>(client);
  }

public:
  MockClient* client = nullptr;
};

#pragma once

#include <gmock/gmock.h>
#include <memory>

#include "client.hpp"

class MockClient : public translator::Client
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
  using translator::Client::Client;

public:
  MOCK_METHOD(translator::SessionPtr,
              create,
              (translator::MessagePtr message),
              (override));

  void send(translator::ScheduleRef sch,
            translator::CoroutineRef co,
            translator::SessionPtr session,
            translator::MessagePtr message)
  {
    message_handler_->on_recv(sch, co, session, message);
  }
};

class MockClientFactory : public translator::ClientFactory
{
public:
  std::unique_ptr<translator::Client> create(
    translator::Client::MessageHandler* handler) override
  {
    client = new MockClient(handler);
    return std::unique_ptr<MockClient>(client);
  }

public:
  MockClient* client = nullptr;
};

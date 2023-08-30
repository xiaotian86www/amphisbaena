#pragma once

#include <gmock/gmock.h>
#include <memory>

#include "plugin/fix_client/client.hpp"

class MockClient : public amphisbaena::Client
{
public:
  using amphisbaena::Client::Client;

public:
  MOCK_METHOD(amphisbaena::SessionPtr,
              create,
              (amphisbaena::MessagePtr message),
              (override));

};

class MockClientFactory : public amphisbaena::ClientFactory
{
public:
  std::unique_ptr<amphisbaena::Client> create(
    amphisbaena::Session::MessageHandler& handler) override
  {
    client = new MockClient(handler);
    return std::unique_ptr<MockClient>(client);
  }

public:
  MockClient* client = nullptr;
};

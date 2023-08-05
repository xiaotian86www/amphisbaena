#pragma once

#include <gtest/gtest.h>
#include <memory>

#include "fixture_schedule.hpp"
#include "mock_server.hpp"
#include "server.hpp"
#include "tool/http_client.hpp"

class Server
  : public FixtureSchedule<testing::TestWithParam<
      std::pair<std::function<std::unique_ptr<amphisbaena::Server>(
                  boost::asio::io_service&,
                  std::shared_ptr<amphisbaena::Schedule>,
                  amphisbaena::Server::MessageHandler*)>,
                std::function<std::unique_ptr<HttpClient>()>>>>
{
public:
  typedef FixtureSchedule<testing::TestWithParam<
    std::pair<std::function<std::unique_ptr<amphisbaena::Server>(
                boost::asio::io_service&,
                std::shared_ptr<amphisbaena::Schedule>,
                amphisbaena::Server::MessageHandler*)>,
              std::function<std::unique_ptr<HttpClient>()>>>>
    Base;

public:
  void SetUp() override
  {
    server = GetParam().first(ios, sch, &message_handler);
    client = GetParam().second();

    Base::SetUp();
  }

  void TearDown() override { Base::TearDown(); }

protected:
  MockServer::MockMessageHandler message_handler;
  std::unique_ptr<amphisbaena::Server> server;
  std::unique_ptr<HttpClient> client;
};

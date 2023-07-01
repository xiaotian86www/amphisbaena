#pragma once

#include "gmock/gmock.h"

#include "detail/uds_server.hpp"

class MockUDSServer : public translator::UDSServer
{
public:
  using UDSServer::UDSServer;

  MOCK_METHOD(void,
              on_data,
              (std::shared_ptr<stream_protocol::socket>,
               std::shared_ptr<translator::Coroutine>,
               std::string_view),
              (override));

  void send(std::shared_ptr<stream_protocol::socket> sock,
            std::shared_ptr<translator::Coroutine> co,
            std::string_view data)
  {
    translator::UDSServer::send(sock, co, data);
  }
};
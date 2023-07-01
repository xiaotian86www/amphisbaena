#pragma once

#include "gmock/gmock.h"

#include "detail/uds_server.hpp"

class MockUDSServer : public translator::UDSServer
{
public:
  using UDSServer::UDSServer;
  
  MOCK_METHOD(void, on_data, (std::string_view), (override));
};
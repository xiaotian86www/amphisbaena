#pragma once

#include "gmock/gmock.h"

#include "server.hpp"

class MockServer : public translator::Server
{
public:
  MOCK_METHOD(std::unique_ptr<translator::Object>,
              call,
              (std::string_view, const translator::Object*),
              (override));
};
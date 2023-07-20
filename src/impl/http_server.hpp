#pragma once

#include "service.hpp"

namespace translator {
class HttpServer : public Service
{
public:
  void start() override;

  void stop() override;

  void send(MessagePtr message) override;
};
}
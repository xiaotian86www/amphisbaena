#pragma once

#include "message.hpp"

namespace translator {
class Service
{
public:
  class MessageHandler
  {
  public:
    virtual ~MessageHandler() = default;

  public:
    virtual void on_message(MessagePtr message) = 0;
  };

  MessageHandler* handler;

public:
  virtual ~Service() = default;

public:
  virtual void start() = 0;

  virtual void stop() = 0;

  virtual void send(MessagePtr message) = 0;
};
}
#pragma once

#include "message.hpp"
#include "schedule.hpp"
#include "session.hpp"
#include <memory>

namespace amphisbaena {
class Client
{
public:
  class MessageHandler
  {
  public:
    virtual ~MessageHandler() = default;

  public:
    virtual void on_recv(ScheduleRef sch,
                         CoroutineRef co,
                         SessionPtr session,
                         MessagePtr message) = 0;
  };

public:
  Client(MessageHandler* message_handler)
    : message_handler_(message_handler)
  {
  }

  virtual ~Client() = default;

public:
  virtual SessionPtr create(MessagePtr message) = 0;

protected:
  MessageHandler* message_handler_ = nullptr;
};

class ClientFactory
{
public:
  virtual ~ClientFactory() = default;

public:
  virtual std::unique_ptr<Client> create(
    Client::MessageHandler* message_handler) = 0;
};

}
#pragma once

#include "message.hpp"
#include "schedule.hpp"
#include "session.hpp"
#include <memory>

namespace amphisbaena {
class Client
{
public:
  Client(Session::MessageHandler& message_handler)
    : message_handler_(message_handler)
  {
  }

  virtual ~Client() = default;

public:
  virtual SessionPtr create(MessagePtr message) = 0;

protected:
  Session::MessageHandler& message_handler_;
};

class ClientFactory
{
public:
  virtual ~ClientFactory() = default;

public:
  virtual std::unique_ptr<Client> create(
    Session::MessageHandler& message_handler) = 0;
};

}
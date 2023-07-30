#pragma once

#include "message.hpp"
#include "schedule.hpp"
#include "session.hpp"

namespace translator {
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

  MessageHandler* message_handler = nullptr;

public:
  virtual ~Client() = default;

public:
  virtual SessionPtr create(MessagePtr message) = 0;
};
}
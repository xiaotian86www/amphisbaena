#pragma once

#include "message.hpp"
#include "schedule.hpp"

namespace translator {
class Session
{
public:
  virtual ~Session() = default;

public:
  virtual void send(MessagePtr data) = 0;
};

typedef std::shared_ptr<Session> SessionPtr;

class Service
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
  virtual ~Service() = default;

public:
  virtual void start() = 0;

  virtual void stop() = 0;

  virtual SessionPtr create(MessagePtr message) = 0;
};
}
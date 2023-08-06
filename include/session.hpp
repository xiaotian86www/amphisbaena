#pragma once

#include "message.hpp"
#include "schedule.hpp"

namespace amphisbaena {
class Session;
typedef std::shared_ptr<Session> SessionPtr;

class Session
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
  virtual ~Session() = default;

public:
  virtual void send(MessagePtr data) = 0;

public:
  std::shared_ptr<Session> up;
};

}
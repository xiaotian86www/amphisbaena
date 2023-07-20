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

class Session
{
public:
  virtual ~Session() = default;

public:
  virtual void send(Environment& env, MessagePtr data) = 0;

};

typedef std::shared_ptr<Session> SessionPtr;

class SessionRef
{
public:
  SessionRef() {}

  SessionRef(std::shared_ptr<Session> session)
    : session_(session)
  {
  }

  SessionRef(std::weak_ptr<Session> session)
    : session_(session)
  {
  }

public:
  void send(Environment& env, MessagePtr data)
  {
    if (auto session = session_.lock()) {
      session->send(env, data);
    }
  }

private:
  std::weak_ptr<Session> session_;
};

class Service2
{
public:
  class MessageHandler
  {
  public:
    virtual ~MessageHandler() = default;

  public:
    virtual void on_recv(Environment& env, MessagePtr message) = 0;
  };

  MessageHandler* handler;

public:
  virtual ~Service2() = default;

public:
  virtual void start() = 0;

  virtual void stop() = 0;

  virtual SessionPtr create(Environment& env, MessagePtr message) = 0;
};
}
#pragma once

#include <string_view>

#include "schedule.hpp"

namespace amphisbaena {

class Connection
{
public:
  Connection(ScheduleRef sch, CoroutineRef co)
    : sch_(sch)
    , co_(co)
  {
  }

  virtual ~Connection() = default;

public:
  virtual void send(std::string_view data) = 0;

  virtual void close() = 0;

protected:
  ScheduleRef sch_;
  CoroutineRef co_;
};

typedef std::shared_ptr<Connection> ConnectionPtr;

class ConnectionRef
{
public:
  ConnectionRef() {}

  ConnectionRef(std::shared_ptr<Connection> connection)
    : connection_(connection)
  {
  }

  ConnectionRef(std::weak_ptr<Connection> connection)
    : connection_(connection)
  {
  }

public:
  void send(std::string_view data)
  {
    if (auto connection = connection_.lock()) {
      connection->send(data);
    }
  }

private:
  std::weak_ptr<Connection> connection_;
};

class Server
{
public:
  class MessageHandler
  {
  public:
    virtual ~MessageHandler() = default;

  public:
    virtual void on_recv(ScheduleRef sch,
                         CoroutineRef co,
                         ConnectionPtr conn,
                         std::string_view data) = 0;
  };

public:
  Server(MessageHandler* message_handler)
    : message_handler_(message_handler)
  {
  }

  virtual ~Server() = default;

protected:
  MessageHandler* message_handler_;
};

class ServerFactory
{
public:
  virtual ~ServerFactory() = default;

public:
  virtual std::unique_ptr<Server> create(
    Server::MessageHandler* message_handler) = 0;
};

}
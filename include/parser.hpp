#pragma once

#include <map>
#include <memory>
#include <string_view>

#include "schedule.hpp"

namespace translator {
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

  virtual std::size_t recv(char* buffer, std::size_t buf_len) = 0;

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

class Parser
{
public:
  Parser(ScheduleRef sch, CoroutineRef co, ConnectionRef conn)
    : sch_(sch)
    , co_(co)
    , conn_(conn)
  {
  }

  virtual ~Parser() = default;

public:
  virtual void on_data(std::string_view data) = 0;

protected:
  ScheduleRef sch_;
  CoroutineRef co_;
  ConnectionRef conn_;
};

class ParserFactory
{
public:
  virtual ~ParserFactory() = default;

public:
  virtual std::shared_ptr<Parser> create(ScheduleRef sch,
                                         CoroutineRef co,
                                         ConnectionRef conn) = 0;
};
} // namespace translator

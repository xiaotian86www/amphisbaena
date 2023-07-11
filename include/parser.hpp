#pragma once

#include <map>
#include <memory>
#include <string_view>

#include "schedule.hpp"

namespace translator {

class Connection
{
public:
  virtual ~Connection() = default;

public:
  virtual void send(ScheduleRef sch,
                    CoroutineRef co,
                    std::string_view data) = 0;
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
  void send(ScheduleRef sch, CoroutineRef co, std::string_view data)
  {
    if (auto connection = connection_.lock()) {
      connection->send(sch, co, data);
    }
  }

private:
  std::weak_ptr<Connection> connection_;
};

class Parser
{
public:
  virtual ~Parser() = default;

public:
  virtual void on_data(ScheduleRef sch,
                       CoroutineRef co,
                       ConnectionRef conn,
                       std::string_view data) = 0;
};

class ParserFactory
{
public:
  virtual ~ParserFactory() = default;

public:
  virtual std::shared_ptr<Parser> create() = 0;
};

// class ServerPool
// {
// public:
//   ServerPool() = default;

// public:
//   void add(std::string_view name, std::shared_ptr<Server> server);

//   Server* get(std::string_view name) const;

// private:
//   std::map<std::string_view, std::shared_ptr<Server>> servers_;
// };

// typedef std::shared_ptr<ServerPool> ServerPoolPtr;
} // namespace translator

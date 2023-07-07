#pragma once

#include <map>
#include <memory>
#include <string_view>

#include "schedule.hpp"
#include "server.hpp"

namespace translator {
class Protocol
{
public:
  virtual ~Protocol() = default;

public:
  virtual void on_data(ScheduleRef sch,
                       CoroutineRef co,
                       std::shared_ptr<Socket> sock,
                       std::string_view data) = 0;
};

class ProtocolFactory
{
public:
  virtual ~ProtocolFactory() = default;

public:
  virtual std::unique_ptr<Protocol> create() = 0;
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

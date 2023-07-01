#pragma once

#include <map>
#include <memory>
#include <string_view>

#include "schedule.hpp"

namespace translator {
class Socket : public std::enable_shared_from_this<Socket>
{
public:
  virtual ~Socket() = default;

public:
  virtual void send(std::shared_ptr<Coroutine> co, std::string_view data) = 0;
};

class Protocol
{
public:
  virtual ~Protocol() = default;

public:
  virtual void on_data(std::shared_ptr<Socket> sock,
                       std::shared_ptr<Coroutine> co,
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

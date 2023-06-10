#pragma once

#include <map>

#include "object.hpp"

namespace translator {
class Server
{
public:
  virtual ~Server() = default;

  virtual std::unique_ptr<Object> call(std::string_view method,
                                       const Object* args) = 0;
};

class ServerPool
{
public:
  ServerPool() = default;

public:
  void add(std::string_view name, std::shared_ptr<Server> server);

  Server* get(std::string_view name) const;

private:
  std::map<std::string_view, std::shared_ptr<Server>> servers_;
};

typedef std::shared_ptr<ServerPool> ServerPoolPtr;
} // namespace translator

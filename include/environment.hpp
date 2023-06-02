#pragma once

#include <memory>
#include <unordered_map>

#include "object.hpp"
#include "server.hpp"

namespace translator {
class Environment
{
public:
  Environment(ObjectFactoryPtr object_factory, ServerPoolPtr server_pool)
    : object_factory_(object_factory)
    , server_pool_(server_pool)
  {
  }

public:
  Object* get_object(std::string_view name);

  void set_object(std::string_view name, ObjectPtr&& object);

  Server* get_server(std::string_view name);

private:
  std::unordered_map<std::string_view, ObjectPtr> objects_;
  ObjectFactoryPtr object_factory_;
  ServerPoolPtr server_pool_;
};
} // namespace translator

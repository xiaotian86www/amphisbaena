#pragma once

#include <memory>
#include <unordered_map>

#include "object.hpp"
#include "protocol.hpp"

namespace translator {
class Environment
{
public:
  Environment(ObjectFactoryPtr object_factory/*, ServerPoolPtr server_pool*/)
    : object_pool_(object_factory)
    // , server_pool_(server_pool)
  {
  }

public:
  Object* get_object(std::string_view name);

  void set_object(std::string_view name, ObjectPtr&& object);

  Protocol* get_server(std::string_view name);

private:
  ObjectPool object_pool_;
  // ServerPoolPtr server_pool_;
};
} // namespace translator

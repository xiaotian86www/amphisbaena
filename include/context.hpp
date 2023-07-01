#pragma once

#include <functional>
#include <map>
#include <sstream>

#include "object.hpp"
#include "server.hpp"

namespace translator {

class Context
{
public:
  using get_object_func_t = std::function<std::unique_ptr<Object>(Context*)>;

public:
  Context()
  {
  }

public:
  Object* get_object(std::string_view name)
  {
    auto it = objects_.find(name);
    if (it != objects_.end())
      return it->second.get();

    auto func_it = object_funcs_.find(name);
    if (func_it != object_funcs_.end()) {
      auto obj = func_it->second(this);
      auto it = objects_.insert(std::make_pair(name, std::move(obj)));
      return it.first->second.get();
    }

    std::stringstream ss;
    ss << "cannot find object '" << name << "'";

    throw new std::runtime_error(ss.str());
  }

  void set_object(std::string_view name, std::unique_ptr<Object>&& obj)
  {
    auto it = objects_.find(name);
    if (it != objects_.end())
      it->second = std::move(obj);
    else
      objects_.insert(std::make_pair(name, std::move(obj)));
  }

  void set_object_func(std::string_view name, get_object_func_t func)
  {
    object_funcs_.insert_or_assign(name, func);
  }

  void set_server(std::string_view name, std::unique_ptr<Protocol>&& server)
  {
    auto it = servers_.find(name);
    if (it != servers_.end())
      it->second = std::move(server);
    else
      servers_.insert(std::make_pair(name, std::move(server)));
  }

  Protocol* get_server(std::string_view name)
  {
    auto it = servers_.find(name);
    if (it != servers_.end())
      return it->second.get();

    std::stringstream ss;
    ss << "cannot find server '" << name << "'";

    throw new std::runtime_error(ss.str());
  }

  void reset() { objects_.clear(); }

private:
  void main_func() {}

private:
  std::map<std::string_view, std::unique_ptr<Object>> objects_;
  std::map<std::string_view, get_object_func_t> object_funcs_;
  std::map<std::string_view, std::unique_ptr<Protocol>> servers_;
};

} // namespace translator

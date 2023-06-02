#include "environment.hpp"

namespace translator {
Object*
Environment::get_object(std::string_view name)
{
  auto it = objects_.find(name);
  if (it != objects_.end())
    return it->second.get();

  auto object = object_factory_->produce(this, name);
  auto it2 = objects_.insert(std::make_pair(name, std::move(object)));

  assert(it2.second);

  return it2.first->second.get();
}

void
Environment::set_object(std::string_view name, ObjectPtr&& object)
{
  objects_[name] = std::move(object);
}

Server*
Environment::get_server(std::string_view name)
{
  return server_pool_->get(name);
}
} // namespace translator

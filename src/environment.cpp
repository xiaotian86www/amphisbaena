#include "environment.hpp"

namespace translator {
Object*
Environment::get_object(std::string_view name)
{
  return object_pool_.get(name, this);
}

void
Environment::set_object(std::string_view name, ObjectPtr&& object)
{
  object_pool_.add(name, std::move(object));
}

Server*
Environment::get_server(std::string_view name)
{
  return server_pool_->get(name);
}
} // namespace translator

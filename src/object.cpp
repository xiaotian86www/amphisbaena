#include "object.hpp"

#include <sstream>

#include "context.hpp"
#include "environment.hpp"
#include "builder.hpp"

namespace translator {

void
ObjectPool::add(std::string_view name, ObjectPtr&& object)
{
  objects_[name] = std::move(object);
}

const Object&
ObjectPool::get(std::string_view name, Environment& env) const
{
  auto it = objects_.find(name);
  if (it != objects_.end())
    return *it->second;

  auto object = Context::get_instance().object_builder->create(name, env);

  return *objects_.insert(std::make_pair(name, std::move(object)))
            .first->second;
}
} // namespace translator

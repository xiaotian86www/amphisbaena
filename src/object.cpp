#include "object.hpp"

#include <sstream>

#include "context.hpp"
#include "environment.hpp"

namespace translator {
void
ObjectFactory::registe(std::string_view name, ctor_function&& func)
{
  ctors_[name] = std::move(func);
}

ObjectPtr
ObjectFactory::create(std::string_view name, Environment& env) const
{
  auto it = ctors_.find(name);
  if (it != ctors_.end()) {
    return it->second(env);
  }

  std::stringstream ss;
  ss << "cannot get object '" << name << "'";

  throw new std::runtime_error(ss.str());
}

void
ObjectPool::add(std::string_view name, ObjectPtr&& object)
{
  objects_[name] = std::move(object);
}

Object&
ObjectPool::get(std::string_view name, Environment& env) const
{
  auto it = objects_.find(name);
  if (it != objects_.end())
    return *it->second;

  auto object = Context::get_instance().object_factory->create(name, env);

  return *objects_.insert(std::make_pair(name, std::move(object)))
            .first->second;
}

} // namespace translator

#include "object.hpp"

#include <sstream>

namespace translator {
void
ObjectFactory::registe(std::string_view name, ctor_func_type&& func)
{
  ctors_[name] = std::move(func);
}

ObjectPtr
ObjectFactory::produce(std::string_view name, Environment* env) const
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

Object*
ObjectPool::get(std::string_view name, Environment* env) const
{
  auto it = objects_.find(name);
  if (it != objects_.end())
    return it->second.get();

  auto object = factory_->produce(name, env);

  return objects_.insert(std::make_pair(name, std::move(object)))
    .first->second.get();
}

} // namespace translator

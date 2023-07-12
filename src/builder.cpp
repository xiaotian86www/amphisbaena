#include "builder.hpp"

#include <memory>
#include <sstream>

#include "context.hpp"
#include "environment.hpp"

namespace translator {
void
ObjectBuilder::registe(std::string_view name, ctor_function&& func)
{
  ctors_[name] = std::move(func);
}

ObjectPtr
ObjectBuilder::create(std::string_view name, Environment& env) const
{
  auto it = ctors_.find(name);
  if (it != ctors_.end()) {
    return it->second(env);
  }

  std::stringstream ss;
  ss << "cannot get object '" << name << "'";

  throw new std::runtime_error(ss.str());
}
}
#include "builder.hpp"

#include <memory>
#include <sstream>

#include "context.hpp"
#include "environment.hpp"
#include "message.hpp"

namespace translator {
void
MessageBuilder::registe(std::string_view name, ctor_function&& func)
{
  ctors_[name] = std::move(func);
}

MessagePtr
MessageBuilder::create(std::string_view name, Environment& env) const
{
  auto it = ctors_.find(name);
  if (it != ctors_.end()) {
    return it->second(env);
  }

  throw NotFoundException(name);
}
}
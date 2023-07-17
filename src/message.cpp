#include "message.hpp"

#include <sstream>

#include "context.hpp"
#include "environment.hpp"
#include "builder.hpp"

namespace translator {

void
MessagePool::add(std::string_view name, MessagePtr&& message)
{
  messages_[name] = std::move(message);
}

const Message&
MessagePool::get(std::string_view name, Environment& env) const
{
  auto it = messages_.find(name);
  if (it != messages_.end())
    return *it->second;

  auto message = Context::get_instance().message_builder->create(name, env);

  return *messages_.insert(std::make_pair(name, std::move(message)))
            .first->second;
}
} // namespace translator

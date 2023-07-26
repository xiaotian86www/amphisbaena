#include "builder.hpp"

#include <memory>
#include <sstream>

#include "environment.hpp"
#include "message.hpp"

namespace translator {
void
MessageBuilder::registe(std::map<std::string_view, ctor_function> ctors)
{
  auto new_ctors =
    ctors_
      ? std::make_shared<
          std::map<std::string, MessageBuilder::ctor_function, std::less<>>>(
          *ctors_)
      : std::make_shared<
          std::map<std::string, MessageBuilder::ctor_function, std::less<>>>();

  for (auto iter = ctors.begin(); iter != ctors.end(); ++iter) {
    new_ctors->insert_or_assign(std::string(iter->first),
                                std::move(iter->second));
  }

  ctors_ = new_ctors;
}

MessagePtr
MessageBuilder::create(Environment& env,
                        std::string_view name,
                        MessagePtr request)
{
  if (auto ctors = ctors_) {
    if (auto it = ctors->find(name); it != ctors->end()) {
      return it->second(env, request);
    }
  }

  throw NotFoundException(name);
}

// std::unordered_map<std::string_view, MessageBuilder::ctor_function>
//   MessageBuilder::ctors_;
std::shared_ptr<
  std::map<std::string, MessageBuilder::ctor_function, std::less<>>>
  MessageBuilder::ctors_;
}
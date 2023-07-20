#pragma once

#include <memory>
#include <string_view>

#include "message.hpp"
#include "schedule.hpp"

namespace translator {

class Environment;

class MessageBuilder
{
public:
  using ctor_prototype = MessagePtr(Environment&, MessagePtr);
  using ctor_function = std::function<ctor_prototype>;

public:
  void registe(std::string_view name, ctor_function&& func);

  MessagePtr create(Environment& env,
                    std::string_view name,
                    MessagePtr request) const;

private:
  std::unordered_map<std::string_view, ctor_function> ctors_;
};

typedef std::shared_ptr<MessageBuilder> MessageBuilderPtr;
}
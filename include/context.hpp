#pragma once

#include <memory>

namespace translator {

class ProcessorFactory;
class ParserFactory;
class MessageBuilder;

class Context
{
private:
  Context() = default;

public:
  static Context& get_instance()
  {
    static Context instance;
    return instance;
  }

public:
  // std::shared_ptr<ProcessorFactory> processor_factory;
  std::shared_ptr<MessageBuilder> message_builder;
  std::shared_ptr<ParserFactory> parser_factory;
};
} // namespace translator

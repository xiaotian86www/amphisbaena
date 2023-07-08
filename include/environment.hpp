#pragma once

#include <memory>
#include <unordered_map>

#include "object.hpp"
#include "parser.hpp"
#include "processor.hpp"

namespace translator {
class Environment
{
public:
  Environment(ObjectFactoryPtr object_factory,
              std::shared_ptr<ParserFactory> parser_factory,
              std::shared_ptr<ProcessorFactory> processor_factory)
    : object_pool_(object_factory)
    , parser_factory_(parser_factory)
    , processor_factory_(processor_factory)
  {
  }

public:
  Object* get_object(std::string_view name);

  void set_object(std::string_view name, ObjectPtr&& object);

  Parser* get_server(std::string_view name);

  std::unique_ptr<Parser> create_parser() { return parser_factory_->create(); }

  std::unique_ptr<Processor> create_processor(std::string_view key)
  {
    return processor_factory_->create(key);
  }

private:
  ObjectPool object_pool_;
  std::shared_ptr<ParserFactory> parser_factory_;
  std::shared_ptr<ProcessorFactory> processor_factory_;
};
} // namespace translator

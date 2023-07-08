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
  Environment(ObjectFactoryPtr object_factory/*, ServerPoolPtr server_pool*/)
    : object_pool_(object_factory)
    // , server_pool_(server_pool)
  {
  }

public:
  Object* get_object(std::string_view name);

  void set_object(std::string_view name, ObjectPtr&& object);

  Parser* get_server(std::string_view name);

private:
  ObjectPool object_pool_;
  std::shared_ptr<ParserFactory> parser_factory_;
  std::shared_ptr<ProcessorFactory> processor_factory_;
  // ServerPoolPtr server_pool_;
};
} // namespace translator

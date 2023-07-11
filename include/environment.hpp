#pragma once

#include <memory>
#include <unordered_map>

#include "object.hpp"
#include "parser.hpp"
#include "processor.hpp"
#include "schedule.hpp"

namespace translator {
class Environment
{
public:
  Environment(ObjectFactoryPtr object_factory)
    : object_pool_(object_factory)
  {
  }

public:
  Object* get_object(std::string_view name);

  void set_object(std::string_view name, ObjectPtr&& object);

private:
  ScheduleRef sch_;
  CoroutineRef co_;
  SessionRef session_;
  ObjectPool object_pool_;
};
} // namespace translator

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
  ScheduleRef sch;
  CoroutineRef co;
  SessionRef session;
  ObjectFactoryPtr object_factory;
  ObjectPool object_pool;
};
} // namespace translator

#pragma once

#include <memory>
#include <unordered_map>

#include "message.hpp"
#include "parser.hpp"
#include "schedule.hpp"
#include "service.hpp"

namespace translator {
class Environment
{
public:
  ScheduleRef sch;
  CoroutineRef co;
  SessionRef up;
  SessionRef down;
  // MessagePool message_pool;
};
} // namespace translator

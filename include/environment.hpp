#pragma once

#include <memory>
#include <unordered_map>

#include "message.hpp"
#include "parser.hpp"
#include "builder.hpp"
#include "schedule.hpp"

namespace translator {
class Environment
{
public:
  ScheduleRef sch;
  CoroutineRef co;
  SessionRef session;
  // MessagePool message_pool;
};
} // namespace translator

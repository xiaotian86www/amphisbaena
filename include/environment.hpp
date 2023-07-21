#pragma once

#include <memory>
#include <unordered_map>

#include "message.hpp"
#include "schedule.hpp"
#include "service.hpp"

namespace translator {
class Environment
{
public:
  ScheduleRef sch;
  CoroutineRef co;
  SessionPtr up;
  SessionPtr down;
  // MessagePool message_pool;
};
} // namespace translator

#pragma once

#include <memory>
#include <unordered_map>

#include "builder.hpp"
#include "message.hpp"
#include "schedule.hpp"
#include "session.hpp"

namespace translator {
class Environment
{
public:
  ScheduleRef sch;
  CoroutineRef co;
  SessionPtr up;
  SessionPtr down;
  MessageBuilderPtr builder;
};
} // namespace translator

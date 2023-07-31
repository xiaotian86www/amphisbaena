#pragma once

#include "builder.hpp"
#include "message.hpp"
#include "schedule.hpp"
#include "session.hpp"

namespace amphisbaena {
class Environment
{
public:
  ScheduleRef sch;
  CoroutineRef co;
  SessionPtr up;
  SessionPtr down;
};
} // namespace amphisbaena

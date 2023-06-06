#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <ucontext.h>
#include <unordered_set>

namespace translator {
class Schedule;
class ScheduleRef;

typedef std::function<void(ScheduleRef)> task;

class Schedule
{
public:
  struct Coroutine;
  struct CoroutineRef
  {
    std::weak_ptr<Schedule::Coroutine> ptr_;
  };

public:
  Schedule();
  virtual ~Schedule();
  Schedule(const Schedule&) = delete;
  Schedule& operator=(const Schedule&) = delete;

public:
  void run();

  void stop();

public:
  void post(task&& func);

  void resume(CoroutineRef co);

  void yield();

  CoroutineRef this_co();

public:
  class Impl;
  std::shared_ptr<Impl> impl_;
};

class ScheduleRef
{
public:
  ScheduleRef(std::weak_ptr<Schedule::Impl> impl);

public:
  void resume(Schedule::CoroutineRef co);

  void yield();

  Schedule::CoroutineRef this_co();

private:
  std::weak_ptr<Schedule::Impl> ptr_;
};

} // namespace translator

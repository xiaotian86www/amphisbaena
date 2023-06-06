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

class Schedule : public std::enable_shared_from_this<Schedule>
{
  friend ScheduleRef;

public:
  struct Coroutine;

  class Impl;

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

public:
  void resume(std::weak_ptr<Coroutine> co);

  void yield();

  std::weak_ptr<Coroutine> this_co();

private:
  std::shared_ptr<Impl> impl_;
};

class ScheduleRef
{
public:
  ScheduleRef(std::weak_ptr<Schedule::Impl> impl);

public:
  void resume(std::weak_ptr<Schedule::Coroutine> co);

  void yield();

  std::weak_ptr<Schedule::Coroutine> this_co();

private:
  std::weak_ptr<Schedule::Impl> impl_;
};

} // namespace translator

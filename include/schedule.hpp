#pragma once

#include <functional>
#include <memory>
#include <chrono>

namespace translator {
class Schedule;
class ScheduleRef;

typedef std::function<void(ScheduleRef)> task;

class Schedule
{
public:
  struct Coroutine;
  typedef std::weak_ptr<Schedule::Coroutine> CoroutinePtr;

  class Worker
  {
  public:
    Worker(Schedule& sch);
    ~Worker();

  private:
    Schedule& sch_;
  };

public:
  Schedule();
  virtual ~Schedule();
  Schedule(const Schedule&) = delete;
  Schedule& operator=(const Schedule&) = delete;

public:
  void run();

  void stop();

  void post(task&& func);

  void resume(CoroutinePtr co);

  void yield();

  void yield_for(int milli);

  CoroutinePtr this_co();

public:
  class Impl;
  std::shared_ptr<Impl> impl_;
};

class ScheduleRef
{
public:
  ScheduleRef(std::weak_ptr<Schedule::Impl> impl);

public:
  void stop();

  void post(task&& func);

  void resume(Schedule::CoroutinePtr co);

  void yield();

  void yield_for(int milli);

  Schedule::CoroutinePtr this_co();

private:
  std::weak_ptr<Schedule::Impl> ptr_;
};

} // namespace translator

#pragma once

#include <chrono>
#include <functional>
#include <memory>

namespace translator {

class CoroutineRef;
class ScheduleRef;

typedef std::function<void(ScheduleRef, CoroutineRef)> task;

class Coroutine;

class CoroutineRef
{
public:
  CoroutineRef(std::weak_ptr<Coroutine> co)
    : impl_(co)
  {
  }

public:
  void yield();

  void yield_for(int milli);

  void resume();

private:
  std::weak_ptr<Coroutine> impl_;
};

class ScheduleImpl;

class Schedule : public std::enable_shared_from_this<Schedule>
{
public:
  Schedule();
  Schedule(const Schedule&) = delete;
  Schedule& operator=(const Schedule&) = delete;

public:
  void run();

  void stop();

  void spawn(task&& func);

  void resume(CoroutineRef co);

  void post(std::function<void()>&& func);

public:
  std::shared_ptr<ScheduleImpl> impl_;
};

class ScheduleRef
{
public:
  ScheduleRef(std::weak_ptr<ScheduleImpl> sch);

public:
  void post(std::function<void()>&& func);

private:
  std::weak_ptr<ScheduleImpl> sch_;
};

} // namespace translator

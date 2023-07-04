#pragma once

#include <chrono>
#include <functional>
#include <memory>

namespace translator {

class Coroutine;
class ScheduleRef;

typedef std::function<void(ScheduleRef, Coroutine)> task;

class CoroutineImpl;

class Coroutine
{
public:
  Coroutine(std::weak_ptr<CoroutineImpl> co)
    : co_(co)
  {
  }

public:
  void yield();

  void yield_for(int milli);

  void resume();

private:
  std::weak_ptr<CoroutineImpl> co_;
};

class Schedule : public std::enable_shared_from_this<Schedule>
{
public:
  Schedule() = default;
  virtual ~Schedule() = default;
  Schedule(const Schedule&) = delete;
  Schedule& operator=(const Schedule&) = delete;

public:
  virtual void run() = 0;

  virtual void stop() = 0;

  virtual void spawn(task&& func) = 0;

  virtual void resume(Coroutine co) = 0;

  virtual void post(std::function<void()>&& func) = 0;
};

class ScheduleRef
{
public:
  ScheduleRef(std::weak_ptr<Schedule> sch)
    : sch_(sch)
  {
  }

public:
  void post(std::function<void()>&& func)
  {
    if (auto sch = sch_.lock()) {
      sch->post(std::move(func));
    }
  }
private:
  std::weak_ptr<Schedule> sch_;
};

} // namespace translator

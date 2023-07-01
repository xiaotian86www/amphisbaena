#pragma once

#include <chrono>
#include <functional>
#include <memory>

namespace translator {
class Schedule;

enum class CoroutineState : int
{
  DEAD = 0,
  READY = 1,
  RUNNING = 2,
  DYING = 3,
  SUSPEND = 4
};

class Coroutine : public std::enable_shared_from_this<Coroutine>
{
public:
  CoroutineState state = CoroutineState::READY;

public:
  Coroutine() = default;
  virtual ~Coroutine() = default;

public:
  virtual void yield() = 0;

  virtual void yield_for(int milli) = 0;

  virtual void resume() = 0;
};

typedef std::function<void(std::shared_ptr<Coroutine>)> task;

class Schedule
{
public:
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

  // void resume(std::weak_ptr<Coroutine> co);

  // void yield();

  // void yield_for(int milli);

  // std::weak_ptr<Coroutine> this_co();

public:
  class Impl;
  std::shared_ptr<Impl> impl_;
};

// class ScheduleRef
// {
// public:
//   explicit ScheduleRef(std::weak_ptr<Schedule::Impl> impl)
//     : ptr_(impl)
//   {
//   }

// public:
//   void stop();

//   void post(task&& func);

//   void resume(std::weak_ptr<Coroutine> co);

//   void yield();

//   void yield_for(int milli);

//   std::weak_ptr<Coroutine> this_co();

// private:
//   std::weak_ptr<Schedule::Impl> ptr_;
// };

} // namespace translator

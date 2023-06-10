#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <ctime>
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

  void post(task&& func);

  void resume(CoroutineRef co);

  void yield();

  template<typename Rep_, typename Period_>
  void yield_for(const std::chrono::duration<Rep_, Period_>& rtime)
  {
    auto s = std::chrono::duration_cast<std::chrono::seconds>(rtime);
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(rtime - s);
    yield_for_({ s.count(), ns.count() });
  }

  CoroutineRef this_co();

private:
  void yield_for_(const timespec& rtime);

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

  void resume(Schedule::CoroutineRef co);

  void yield();

  template<typename Rep_, typename Period_>
  void yield_for(const std::chrono::duration<Rep_, Period_>& rtime)
  {
    auto s = std::chrono::duration_cast<std::chrono::seconds>(rtime);
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(rtime - s);
    yield_for_({ s.count(), ns.count() });
  }

  Schedule::CoroutineRef this_co();

private:
  void yield_for_(const timespec& rtime);

private:
  std::weak_ptr<Schedule::Impl> ptr_;
};

} // namespace translator

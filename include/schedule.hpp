#pragma once

#include <boost/asio/io_service.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <unordered_set>

namespace translator {

class CoroutineRef;
class ScheduleRef;

typedef std::function<void(ScheduleRef, CoroutineRef)> task;

class Coroutine;

class CoroutineRef
{
  friend class Schedule;

public:
  CoroutineRef(std::weak_ptr<Coroutine> co)
    : co_(co)
  {
  }

public:
  void yield();

  void yield_for(int milli);

public:
  std::weak_ptr<Coroutine> co_;
};

class Schedule : public std::enable_shared_from_this<Schedule>
{
public:
  Schedule(boost::asio::io_service& ios);
  ~Schedule() = default;
  Schedule(const Schedule&) = delete;
  Schedule& operator=(const Schedule&) = delete;

public:
  void spawn(task&& func);

  void resume(CoroutineRef co);

private:
  boost::asio::io_service& ios_;
  std::unordered_set<std::shared_ptr<Coroutine>> cos_;
};

class ScheduleRef
{
public:
  ScheduleRef(std::weak_ptr<Schedule> sch);

public:
  void resume(CoroutineRef co);

private:
  std::weak_ptr<Schedule> sch_;
};

} // namespace translator

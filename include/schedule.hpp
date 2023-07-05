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
public:
  CoroutineRef(std::weak_ptr<Coroutine> co)
    : co_(co)
  {
  }

public:
  void yield();

  void yield_for(int milli);

  void resume();

private:
  std::weak_ptr<Coroutine> co_;
};

class Schedule : public std::enable_shared_from_this<Schedule>
{
public:
  Schedule() = default;
  ~Schedule() = default;
  Schedule(const Schedule&) = delete;
  Schedule& operator=(const Schedule&) = delete;

public:
  void run();

  void stop();

  void spawn(task&& func);

  void resume(CoroutineRef co);

  void post(std::function<void()>&& func);

public:
  boost::asio::io_service& io_service() { return ios_; }

private:
  boost::asio::io_service ios_;
  std::unordered_set<std::shared_ptr<Coroutine>> cos_;
};

class ScheduleRef
{
public:
  ScheduleRef(std::weak_ptr<Schedule> sch);

public:
  void post(std::function<void()>&& func);

private:
  std::weak_ptr<Schedule> sch_;
};

} // namespace translator

#pragma once

#include <chrono>
#include <functional>
#include <memory>

namespace translator {
class Coroutine : public std::enable_shared_from_this<Coroutine>
{
public:
  Coroutine() = default;
  virtual ~Coroutine() = default;

public:
  virtual void yield() = 0;

  virtual void yield_for(int milli) = 0;

  virtual void resume() = 0;
};

typedef std::function<void(Coroutine*)> task;

class Schedule : public std::enable_shared_from_this<Schedule>
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
  Schedule() = default;
  virtual ~Schedule() = default;
  Schedule(const Schedule&) = delete;
  Schedule& operator=(const Schedule&) = delete;

public:
  virtual void run() = 0;

  virtual void stop() = 0;

  virtual void post(task&& func) = 0;
};

} // namespace translator

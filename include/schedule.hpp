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

public:
  class Impl;
  std::shared_ptr<Impl> impl_;
};

} // namespace translator

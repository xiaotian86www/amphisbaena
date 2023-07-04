#pragma once

#include <chrono>
#include <functional>
#include <memory>

namespace translator {

class CoroutineRef;
class Schedule;

typedef std::function<void(std::weak_ptr<Schedule>, CoroutineRef)> task;

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

class CoroutineRef
{
public:
  CoroutineRef(std::weak_ptr<Coroutine> co)
    : co_(co)
  {
  }
  
public:
  void yield()
  {
    auto co = co_.lock().get();
    co->yield();
  }

  void yield_for(int milli)
  {
    auto co = co_.lock().get();
    co->yield_for(milli);
  }

  void resume()
  {
    if (auto co = co_.lock()) {
      co->resume();
    }
  }

private:
  std::weak_ptr<Coroutine> co_;
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
};

} // namespace translator

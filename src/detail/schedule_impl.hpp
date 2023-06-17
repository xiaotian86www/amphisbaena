#pragma once

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>
#include <queue>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <ucontext.h>
#include <unordered_set>
#include <vector>

#include "schedule.hpp"

#define STACK_SIZE (1024 * 1024)
#define EPOLL_MAX_EVENTS 16

namespace translator {

enum class CoroutineState : int
{
  DEAD = 0,
  READY = 1,
  RUNNING = 2,
  SUSPEND = 3
};

struct CoContext
{
  std::vector<char> stack_;
  ucontext_t uct_;
  CoroutineState state_ = CoroutineState::READY;
};

struct CoTimer
{
  int timeout = 0;
  Schedule::CoroutinePtr co;
};

typedef std::shared_ptr<CoTimer> CoTimerPtr;

struct CoTimerPtrGreater
{
  bool operator()(const CoTimerPtr& left, const CoTimerPtr& right) const
  {
    return left->timeout > right->timeout;
  }
};

struct Schedule::Coroutine
{
  task func;
  CoContext context;
  CoTimerPtr timer;
};

class Schedule::Impl : public std::enable_shared_from_this<Schedule::Impl>
{
  friend void co_func_wrapper(uint32_t low32, uint32_t high32);

public:
  Impl(Schedule* sch);

  ~Impl();

public:
  void run();

  void stop();

  void post(task&& func);

  void yield();

  void yield_for(int milli);

  void resume(CoroutinePtr co);

  static void co_func_wrapper(uint32_t low32, uint32_t high32);

public:
  CoroutinePtr this_co()
  {
    assert(running_);
    return running_;
  }

  void increase()
  {
    ++co_count_;
    eventfd_write(event_fd, 1);
  }

  void decrease()
  {
    --co_count_;
    eventfd_write(event_fd, 1);
  }

private:
  void co_func();

  int run_timer();

  int run_once();

private:
  CoContext context_;
  std::unordered_set<std::shared_ptr<Coroutine>> cos_;
  std::shared_ptr<Coroutine> running_;
  std::queue<CoroutinePtr> running_cos_;
  std::atomic<int32_t> co_count_;
  std::priority_queue<CoTimerPtr, std::vector<CoTimerPtr>, CoTimerPtrGreater>
    timers_que_;
  std::mutex mtx_;

  int epoll_fd = 0;
  int event_fd = 0;
};

}
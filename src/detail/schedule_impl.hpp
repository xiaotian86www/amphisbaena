#pragma once

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

struct CoContext
{
  std::vector<char> stack_;
  ucontext_t uct_;
};

struct CoTimer
{
  timespec timeout;
  Schedule::CoroutinePtr co;
};

typedef std::shared_ptr<CoTimer> CoTimerPtr;

struct CoTimerPtrGreater
{
  bool operator()(const CoTimerPtr& left, const CoTimerPtr& right) const
  {
    return left->timeout.tv_sec > right->timeout.tv_sec ||
           (left->timeout.tv_sec == right->timeout.tv_sec &&
            left->timeout.tv_nsec > right->timeout.tv_nsec);
  }
};

struct Schedule::Coroutine
{
  Coroutine() = default;

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

  void yield_for(const timespec& duration);

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
    std::lock_guard<std::mutex> lg(mtx_);
    ++co_count_;
    cv_.notify_all();
  }

  void decrease()
  {
    std::lock_guard<std::mutex> lg(mtx_);
    --co_count_;
    cv_.notify_all();
  }

private:
  void co_func();

  template<typename Func_>
  void check_timer(std::unique_lock<std::mutex>& ul, Func_&& pred);

  bool run_once(std::unique_lock<std::mutex>& ul);

private:
  CoContext context_;
  std::unordered_set<std::shared_ptr<Coroutine>> cos_;
  std::shared_ptr<Coroutine> running_;
  std::queue<CoroutinePtr> running_cos_;
  int32_t co_count_ = 0;
  std::priority_queue<CoTimerPtr, std::vector<CoTimerPtr>, CoTimerPtrGreater>
    timers_que_;
  std::mutex mtx_;
  std::condition_variable cv_;

  int epoll_fd = 0;
  int event_fd = 0;
};

}
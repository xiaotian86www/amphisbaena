#pragma once

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <ucontext.h>
#include <vector>

#define STACK_SIZE (1024 * 1024)

namespace translator {
typedef std::function<void()> task;

class Schedule
{
public:
  Schedule();
  ~Schedule();

public:
  int create(task&& func);

  void destroy(int id);

  void yield();

  void resume(int id);

  int running_id();

  static Schedule* current_schedule();

private:
  void th_func();

private:
  class Coroutine;

  char stack_[STACK_SIZE];
  ucontext_t main_;
  std::priority_queue<int, std::vector<int>, std::greater<int>> free_ids_;
  std::vector<std::unique_ptr<Coroutine>> cos_;
  std::mutex cos_mtx_;
  Coroutine* running_ = nullptr;
  std::queue<Coroutine*> running_cos_;
  std::mutex running_cos_mtx_;
  std::condition_variable running_cos_cv_;
  std::thread th_;
  bool th_running_ = true;
};

template<typename Tp_>
class Promise;

template<typename Tp_>
class Future
{
  friend class Promise<Tp_>;

public:
  Tp_&& get()
  {
    sch_ = Schedule::current_schedule();
    co_id_ = sch_->running_id();

    sch_->yield();
    return std::move(value_);
  }

private:
  Tp_ value_;
  Schedule* sch_ = nullptr;
  int co_id_;
};

template<typename Tp_>
class Promise
{
public:
  void set(Tp_&& value)
  {
    ft_.value_ = std::move(value);

    ft_.sch_->resume(ft_.co_id_);
  }

private:
  Future<Tp_> ft_;
};

} // namespace translator

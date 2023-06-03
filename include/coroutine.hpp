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
  struct Context
  {
    std::vector<char> stack_;
    ucontext_t uct_;
  };

  struct Coroutine
  {
    Context context;
    task func;
    int id;
  };

public:
  Schedule();
  ~Schedule();

public:
  void post(task&& func);

  void wake(Coroutine* co);

  static void yield();

  static Coroutine* this_co();

  static Schedule* this_sch();

private:
  void th_func();

  static void co_func();

  Coroutine* co_create(task&& func);

  void co_destroy(Coroutine* co);

private:
  Context context_;
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
    sch_ = Schedule::this_sch();
    co_ = Schedule::this_co();

    Schedule::yield();
    return std::move(value_);
  }

private:
  Tp_ value_;
  Schedule* sch_ = nullptr;
  Schedule::Coroutine* co_ = nullptr;
};

template<typename Tp_>
class Promise
{
public:
  void set(Tp_&& value)
  {
    ft_.value_ = std::move(value);

    ft_.sch_->wake(ft_.co_);
  }

private:
  Future<Tp_> ft_;
};

} // namespace translator

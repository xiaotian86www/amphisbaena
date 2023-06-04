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

  void resume(Coroutine* co);

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
} // namespace translator

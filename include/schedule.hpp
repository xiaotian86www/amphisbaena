#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <ucontext.h>
#include <unordered_set>

namespace translator {
class Schedule;

typedef std::function<void(Schedule*)> task;

class Schedule : public std::enable_shared_from_this<Schedule>
{
public:
  struct Context
  {
    std::vector<char> stack_;
    ucontext_t uct_;
  };

  struct Coroutine;

public:
  Schedule();
  ~Schedule();

public:
  void run();

  void stop();

public:
  void post(task&& func);

public:
  void resume(std::weak_ptr<Coroutine> co);

  void yield();

  std::weak_ptr<Coroutine> this_co();

private:
  static void co_func(uint32_t low32, uint32_t high32);

  std::weak_ptr<Coroutine> co_create(task&& func);

  void co_destroy(std::shared_ptr<Coroutine> co);

private:
  Context context_;
  std::unordered_set<std::shared_ptr<Coroutine>> cos_;
  std::mutex cos_mtx_;
  std::shared_ptr<Coroutine> running_;
  std::queue<std::shared_ptr<Coroutine>> running_cos_;
  std::mutex running_cos_mtx_;
  std::condition_variable running_cos_cv_;
  std::atomic<int> co_count_;
  // std::thread th_;
  bool th_running_ = true;
};
} // namespace translator

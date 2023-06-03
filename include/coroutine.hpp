#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <ucontext.h>
#include <vector>
#include <condition_variable>

#define STACK_SIZE (1024 * 1024)

namespace translator {
class Coroutine;
typedef std::unique_ptr<Coroutine> CoroutinePtr;
typedef std::function<void()> co_func;

class Schedule
{
public:
  Schedule();
  ~Schedule();

public:
  int create(co_func&& func);

  void destroy(int id);

private:
  void th_func();

public:
  char stack_[STACK_SIZE];
  ucontext_t main_;
  Coroutine* running_ = nullptr;
  std::priority_queue<int, std::vector<int>, std::greater<int>> free_ids_;
  std::vector<CoroutinePtr> cos_;
  std::queue<Coroutine*> running_cos_;
  std::mutex running_cos_mtx_;
  std::condition_variable running_cos_cv_;
  std::thread th_;
  bool th_running_ = true;
};

void co_yield ();

void
co_resume(int id);

int
co_id();

} // namespace translator

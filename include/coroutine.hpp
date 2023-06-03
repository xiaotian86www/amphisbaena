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
typedef std::function<void()> co_func;

class Schedule
{
public:
  Schedule();
  ~Schedule();

public:
  int create(co_func&& func);

  void destroy(int id);

  void yield();

  void resume(int id);

  int running_id();

private:
  void th_func();

private:
  class Coroutine;

  char stack_[STACK_SIZE];
  ucontext_t main_;
  Coroutine* running_ = nullptr;
  std::priority_queue<int, std::vector<int>, std::greater<int>> free_ids_;
  std::vector<std::unique_ptr<Coroutine>> cos_;
  std::queue<Coroutine*> running_cos_;
  std::mutex running_cos_mtx_;
  std::condition_variable running_cos_cv_;
  std::thread th_;
  bool th_running_ = true;
};

// void co_yield ();

// void
// co_resume(int id);

// int
// co_id();

class Promise
{

};

class Future
{

};

} // namespace translator

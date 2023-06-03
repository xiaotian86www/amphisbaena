#pragma once

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
class Coroutine;
class Schedule;
struct CoroutineDeleter
{
  void operator()(Coroutine* co) const;
};

typedef std::unique_ptr<Coroutine, CoroutineDeleter> CoroutinePtr;
typedef std::function<void()> coroutine_func;

class Schedule
{
public:
  Schedule();
  ~Schedule();

public:
  CoroutinePtr create(coroutine_func&& func);

  void destroy(Coroutine* co);

public:
  char stack_[STACK_SIZE];
  ucontext_t main_;
  Coroutine* running_ = nullptr;
  std::priority_queue<int, std::vector<int>, std::greater<int>> free_ids_;
  std::vector<Coroutine*> cos_;
  std::thread th_;
};

class Coroutine
{
public:
  enum class StatusEnum : int
  {
    COROUTINE_DEAD = 0,
    COROUTINE_READY = 1,
    COROUTINE_RUNNING = 2,
    COROUTINE_SUSPEND = 3
  };

public:
  Coroutine(int id, coroutine_func&& func);

  ~Coroutine();

public:
  void resume();

  void yield();

public:
  StatusEnum status() { return status_; }

  int id() { return id_; }

private:
  static void mainfunc(uint32_t low32, uint32_t hi32);

private:
  std::vector<char> stack_;
  coroutine_func func_;
  ucontext_t ctx_;
  StatusEnum status_ = StatusEnum::COROUTINE_READY;
  int id_;
};

void co_yield ();

void
co_resume(int id);

int
co_id();

} // namespace translator

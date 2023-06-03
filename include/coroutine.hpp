#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <ucontext.h>
#include <vector>

#define STACK_SIZE (1024 * 1024)

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

  using coroutine_func = std::function<void(Coroutine*)>;

public:
  Coroutine(coroutine_func&& func);

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
  StatusEnum status_;
  int id_;
};

void
co_yield();

void
co_resume(int id);

int
co_id();

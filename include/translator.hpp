#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <stack>
#include <thread>
#include <vector>

#include "context.hpp"

namespace translator {

class Translator
{
public:
  Translator() {}

  ~Translator()
  {
    is_running_ = false;
    if (th_.joinable())
      th_.join();
  }

public:
  void push(task&& task)
  {
    Context* co_context = nullptr;
    {
      std::unique_lock<std::mutex> ul(idle_co_mtx_);
      idle_co_cv_.wait(ul, [this] { return !idle_co_.empty(); });

      co_context = idle_co_.top();
      co_context->set_task(std::move(task));
      idle_co_.pop();
    }

    {
      std::unique_lock<std::mutex> ul(runnable_co_mtx_);
      runnable_co_.push(co_context);
      runnable_co_cv_.notify_all();
    }
  }

private:
  void th_func()
  {
    while (is_running_) {
      Context* co_context = nullptr;
      {
        std::unique_lock<std::mutex> ul(runnable_co_mtx_);
        runnable_co_cv_.wait(ul, [this] { return !runnable_co_.empty(); });

        co_context = runnable_co_.front();
        runnable_co_.pop();
      }

      coroutine_resume(schedule_, co_context->co_id());
    }
  }

  void co_func(Context* context)
  {
    while (true) {
      if (queue_.empty()) {
        coroutine_yield(schedule_);
        continue;
      }

      // do something

      // runnable
      {
        std::unique_lock<std::mutex> ul(idle_co_mtx_);
        idle_co_.push(context);
        idle_co_cv_.notify_all();
      }
    }
  }

private:
  std::stack<Context*> idle_co_;
  std::mutex idle_co_mtx_;
  std::condition_variable idle_co_cv_;

  std::queue<Context*> runnable_co_;
  std::mutex runnable_co_mtx_;
  std::condition_variable runnable_co_cv_;

  std::thread th_;
  bool is_running_ = true;
  struct schedule* schedule_ = coroutine_open();
};
} // namespace translator

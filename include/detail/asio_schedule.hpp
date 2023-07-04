#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/coroutine2/coroutine.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "schedule.hpp"

using namespace boost::coroutines2;

namespace translator {

enum class CoroutineState : int
{
  COROUTINE_DEAD = 0,
  COROUTINE_READY = 1,
  COROUTINE_SUSPEND = 2,
  COROUTINE_RUNNING = 3
};

class ScheduleImpl;

class CoroutineImpl : public std::enable_shared_from_this<CoroutineImpl>
{
public:
  CoroutineImpl(boost::asio::io_service& ios, ScheduleRef sch, task&& fn);

  ~CoroutineImpl();

public:
  void yield();

  void yield_for(int milli);

  void resume();

  void do_yield();

  void do_resume();

private:
  ScheduleRef sch_;
  boost::asio::steady_timer timer_;
  coroutine<void>::push_type ps_;
  coroutine<void>::pull_type* pl_ = nullptr;
  CoroutineState state_ = CoroutineState::COROUTINE_READY;
};

class ScheduleImpl : public std::enable_shared_from_this<ScheduleImpl>
{
public:
  ScheduleImpl() = default;
  ~ScheduleImpl() = default;

public:
  void run();

  void stop();

  void spawn(task&& fn);

  void resume(Coroutine co);

  void post(std::function<void()>&& fn);

public:
  boost::asio::io_service& io_service() { return ios_; }

private:
  boost::asio::io_service ios_;
  std::unordered_set<std::shared_ptr<CoroutineImpl>> cos_;
};

}
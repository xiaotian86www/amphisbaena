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

class AsioSchedule;

class AsioCoroutine : public Coroutine
{
public:
  AsioCoroutine(boost::asio::io_service& ios,
                ScheduleRef sch,
                task&& fn);

  ~AsioCoroutine() override;

public:
  void yield() override;

  void yield_for(int milli) override;

  void resume() override;

  void do_yield();

  void do_resume();

private:
  ScheduleRef sch_;
  boost::asio::steady_timer timer_;
  coroutine<void>::push_type ps_;
  coroutine<void>::pull_type* pl_ = nullptr;
  CoroutineState state_ = CoroutineState::COROUTINE_READY;
};

class AsioSchedule : public Schedule
{
public:
  AsioSchedule() = default;
  ~AsioSchedule() override = default;

public:
  void run() override;

  void stop() override;

  void spawn(task&& fn) override;

  void resume(CoroutineRef co) override;

  void post(std::function<void()>&& fn) override;

public:
  boost::asio::io_service& io_service() { return ios_; }

private:
  boost::asio::io_service ios_;
  std::unordered_set<std::shared_ptr<Coroutine>> cos_;
};

}
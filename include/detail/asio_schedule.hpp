#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/coroutine2/coroutine.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "schedule.hpp"

using namespace boost::coroutines2;

namespace translator {

class AsioSchedule;

class AsioCoroutine : public Coroutine
{
public:
  AsioCoroutine(std::shared_ptr<AsioSchedule> sch);

public:
  void spawn(task&& fn);

public:
  void yield() override;

  void yield_for(int milli) override;

  void resume() override;

private:
  std::weak_ptr<AsioSchedule> sch_;
  boost::asio::steady_timer timer_;
  coroutine<void>::pull_type pl_;
  coroutine<void>::push_type* ps_;
};

class AsioSchedule : public Schedule
{
public:
  AsioSchedule() = default;
  ~AsioSchedule() override = default;

public:
  void run() override;

  void stop() override;

  void post(task&& fn) override;

public:
  boost::asio::io_service& io_service() { return ios_; }

private:
  boost::asio::io_service ios_;
};

}
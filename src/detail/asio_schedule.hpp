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

struct AsioCoroutine : Schedule::Coroutine
{
  template<typename Fn>
  AsioCoroutine(boost::asio::io_service& ios, Fn&& fn)
    : timer(ios)
    , resume(
        [this, fn = std::move(fn)](coroutine<void>::pull_type& pl) mutable {
          yield = &pl;
          state = Schedule::CoroutineState::RUNNING;
          fn();
          state = Schedule::CoroutineState::DEAD;
        })
    , yield(nullptr)
  {
  }

  boost::asio::steady_timer timer;
  coroutine<void>::push_type resume;
  coroutine<void>::pull_type* yield;
};

class Schedule::Impl : public std::enable_shared_from_this<Schedule::Impl>
{
public:
  Impl();

public:
  void run();

  void stop();

  void post(task&& fn);

  void yield();

  void yield_for(int milli);

  void resume(std::weak_ptr<Schedule::Coroutine> co);

public:
  std::weak_ptr<Coroutine> this_co()
  {
    assert(running_co_);
    return running_co_;
  }

  boost::asio::io_service& io_service()
  {
    return ios_;
  }

private:
  void do_resume(std::shared_ptr<AsioCoroutine> co);

private:
  boost::asio::io_service ios_;
  std::unordered_set<std::shared_ptr<AsioCoroutine>> cos_;
  std::shared_ptr<AsioCoroutine> running_co_;
};

}
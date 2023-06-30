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
    , ps_([this, fn = std::move(fn)](coroutine<void>::pull_type& pl) mutable {
      pl_ = &pl;
      state = Schedule::CoroutineState::RUNNING;
      fn();
      state = Schedule::CoroutineState::DEAD;
    })
    , pl_(nullptr)
  {
  }

  void yield() override
  {
    if (pl_ && *pl_)
      (*pl_)();
  }

  void yield_for(int milli) override
  {
    if (pl_ && *pl_) {
      timer.expires_from_now(std::chrono::milliseconds(milli));
      timer.async_wait([this](boost::system::error_code ec) mutable {
        if (!ec)
          resume();
      });
      (*pl_)();
    }
  }

  void resume() override
  {
    boost::system::error_code ec;
    timer.cancel(ec);

    ps_();
  }

  boost::asio::steady_timer timer;
  coroutine<void>::push_type ps_;
  coroutine<void>::pull_type* pl_;
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

  void resume(std::weak_ptr<Coroutine> co);

public:
  std::weak_ptr<Coroutine> this_co()
  {
    assert(running_co_);
    return running_co_;
  }

  boost::asio::io_service& io_service() { return ios_; }

private:
  void do_resume(std::shared_ptr<Coroutine> co);

private:
  boost::asio::io_service ios_;
  std::unordered_set<std::shared_ptr<Coroutine>> cos_;
  std::shared_ptr<Coroutine> running_co_;
};

}
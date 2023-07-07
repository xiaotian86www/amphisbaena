#include "schedule.hpp"

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/coroutine2/coroutine.hpp>
#include <boost/system/error_code.hpp>
#include <functional>
#include <memory>
#include <unordered_map>

using namespace boost::coroutines2;

namespace translator {

class Coroutine : public std::enable_shared_from_this<Coroutine>
{
public:
  Coroutine(boost::asio::io_service& ios, ScheduleRef sch, task&& fn);

  ~Coroutine();

public:
  void yield();

  void yield_for(int milli);

  bool resume();

  void do_yield();

private:
  ScheduleRef sch_;
  boost::asio::steady_timer timer_;
  coroutine<void>::push_type ps_;
  coroutine<void>::pull_type* pl_ = nullptr;
};

Coroutine::Coroutine(boost::asio::io_service& ios, ScheduleRef sch, task&& fn)
  : sch_(sch)
  , timer_(ios)
  , ps_([this, fn = std::move(fn)](coroutine<void>::pull_type& pl) mutable {
    pl_ = &pl;
    fn(sch_, weak_from_this());
  })
  , pl_(nullptr)
{
}

Coroutine::~Coroutine() {}

void
Coroutine::yield()
{
  do_yield();
}

void
Coroutine::yield_for(int milli)
{
  timer_.expires_from_now(std::chrono::milliseconds(milli));
  timer_.async_wait(
    [this, co = weak_from_this()](boost::system::error_code ec) mutable {
      if (ec)
        return;

      sch_.resume(co);
    });

  do_yield();
}

bool
Coroutine::resume()
{
  boost::system::error_code ec;
  timer_.cancel(ec);

  assert(ps_);
  ps_();

  return !ps_;
}

void
Coroutine::do_yield()
{
  assert(pl_);
  assert(*pl_);
  (*pl_)();
}

void
Schedule::run()
{
  try {
    ios_.run();
  } catch (...) {
  }
}

void
Schedule::stop()
{
  if (!ios_.stopped())
    ios_.stop();
}

void
Schedule::spawn(task&& fn)
{
  auto co = std::make_shared<Coroutine>(ios_, weak_from_this(), std::move(fn));

  cos_.insert(co);

  co->resume();
}

void
Schedule::resume(CoroutineRef co)
{
  ios_.post([this, co]() mutable {
    if (auto c = co.co_.lock(); c && c->resume()) {
      cos_.erase(c);
    }
  });
}

void
CoroutineRef::yield()
{
  auto co = co_.lock().get();
  assert(co);
  co->yield();
}

void
CoroutineRef::yield_for(int milli)
{
  auto co = co_.lock().get();
  assert(co);
  co->yield_for(milli);
}

ScheduleRef::ScheduleRef(std::weak_ptr<Schedule> sch)
  : sch_(sch)
{
}

void
ScheduleRef::resume(CoroutineRef co)
{
  if (auto sch = sch_.lock()) {
    sch->resume(co);
  }
}
}
#include "schedule.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/coroutine2/coroutine.hpp>
#include <boost/system/error_code.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

using namespace boost::coroutines2;

namespace amphisbaena {

class Coroutine : public std::enable_shared_from_this<Coroutine>
{
public:
  Coroutine(boost::asio::io_service& ios, ScheduleRef sch, task&& fn)
    : sch_(sch)
    , timer_(ios)
    , ps_([this, fn = std::move(fn)](coroutine<void>::pull_type& pl) mutable {
      pl_ = &pl;
      fn(sch_, weak_from_this());
    })
    , pl_(nullptr)
  {
  }

  virtual ~Coroutine() = default;

public:
  void yield() { do_yield(); }

  void yield_for(int milli)
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

  bool resume()
  {
    boost::system::error_code ec;
    timer_.cancel(ec);

    assert(ps_);
    ps_();

    return !ps_;
  }

  void do_yield()
  {
    assert(pl_);
    assert(*pl_);
    (*pl_)();
  }

private:
  ScheduleRef sch_;
  boost::asio::steady_timer timer_;
  coroutine<void>::push_type ps_;
  coroutine<void>::pull_type* pl_ = nullptr;
};

Schedule::Schedule(boost::asio::io_service& ios)
  : ios_(ios)
{
}

void
Schedule::spawn(task fn)
{
  auto co = std::make_shared<Coroutine>(ios_, weak_from_this(), std::move(fn));
  {
    std::lock_guard<std::mutex> lg(cos_mtx_);
    cos_.insert(co);
  }

  resume(co);
}

void
Schedule::resume(CoroutineRef co)
{
  ios_.post([this, co]() mutable {
    if (auto c = co.co_.lock(); c && c->resume()) {
      std::lock_guard<std::mutex> lg(cos_mtx_);
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

void
ScheduleRef::resume(CoroutineRef co)
{
  if (auto sch = sch_.lock()) {
    sch->resume(co);
  }
}
}
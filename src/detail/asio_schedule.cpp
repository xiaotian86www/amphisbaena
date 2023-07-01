#include "asio_schedule.hpp"

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/coroutine/exceptions.hpp>
#include <boost/system/errc.hpp>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>

namespace translator {

AsioCoroutine::AsioCoroutine(std::shared_ptr<Schedule::Impl> sch)
  : sch_(sch)
  , timer_(sch->io_service())
  , ps_([](coroutine<void>::pull_type&) {})
  , pl_(nullptr)
{
}

void
AsioCoroutine::spawn(std::shared_ptr<Schedule::Impl> sch, task&& fn)
{
  auto co = std::make_shared<AsioCoroutine>(sch);

  co->ps_ = coroutine<void>::push_type(
    [co, fn = std::move(fn)](coroutine<void>::pull_type& pl) mutable {
      co->pl_ = &pl;
      // state = CoroutineState::RUNNING;
      fn(co);
      // state = CoroutineState::DEAD;
    });

  co->resume();
}

void
AsioCoroutine::yield()
{
  if (pl_ && *pl_)
    (*pl_)();
}

void
AsioCoroutine::yield_for(int milli)
{
  if (pl_ && *pl_) {
    timer_.expires_from_now(std::chrono::milliseconds(milli));
    timer_.async_wait([this](boost::system::error_code ec) mutable {
      if (!ec)
        resume();
    });
    (*pl_)();
  }
}

void
AsioCoroutine::resume()
{
  auto sch = sch_.lock();
  if (!sch)
    return;

  sch->io_service().post(
    [co = std::static_pointer_cast<AsioCoroutine>(shared_from_this())] {
      boost::system::error_code ec;
      co->timer_.cancel(ec);
      if (co->ps_)
        co->ps_();
    });
}

void
Schedule::Impl::run()
{
  try {
    ios_.run();
  } catch (...) {
  }
}

void
Schedule::Impl::stop()
{
  ios_.stop();
}

void
Schedule::Impl::post(task&& fn)
{
  ios_.post([this, fn = std::move(fn)]() mutable {
    AsioCoroutine::spawn(shared_from_this(), std::move(fn));
  });
}

}
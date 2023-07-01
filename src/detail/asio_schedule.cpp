#include "detail/asio_schedule.hpp"

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/coroutine/exceptions.hpp>
#include <boost/system/errc.hpp>
#include <functional>
#include <memory>
#include <unordered_map>

namespace translator {

AsioCoroutine::AsioCoroutine(std::shared_ptr<AsioSchedule> sch)
  : sch_(sch)
  , timer_(sch->io_service())
  , pl_([](coroutine<void>::push_type&) {})
  , ps_(nullptr)
{
}

AsioCoroutine::~AsioCoroutine()
{
}

void
AsioCoroutine::spawn(task&& fn)
{
  pl_ = coroutine<void>::pull_type(
    [co = std::static_pointer_cast<AsioCoroutine>(shared_from_this()),
     fn = std::move(fn)](coroutine<void>::push_type& ps) mutable {
      co->ps_ = &ps;
      fn(co);
    });
}

void
AsioCoroutine::yield()
{
  if (ps_ && *ps_)
    (*ps_)();
}

void
AsioCoroutine::yield_for(int milli)
{
  if (ps_ && *ps_) {
    timer_.expires_from_now(std::chrono::milliseconds(milli));
    timer_.async_wait([this](boost::system::error_code ec) mutable {
      if (!ec)
        resume();
    });
    (*ps_)();
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
      if (co->pl_)
        co->pl_();
    });
}

void
AsioSchedule::run()
{
  try {
    ios_.run();
  } catch (...) {
  }
}

void
AsioSchedule::stop()
{
  if (!ios_.stopped())
    ios_.stop();
}

void
AsioSchedule::post(task&& fn)
{
  ios_.post([this, fn = std::move(fn)]() mutable {
    auto co = std::make_shared<AsioCoroutine>(
      std::static_pointer_cast<AsioSchedule>(shared_from_this()));
    co->spawn(std::move(fn));
  });
}

}
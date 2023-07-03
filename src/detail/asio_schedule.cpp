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

AsioCoroutine::~AsioCoroutine() {}

void
AsioCoroutine::spawn(task&& fn)
{
  assert(state_ == CoroutineState::COROUTINE_READY);
  state_ = CoroutineState::COROUTINE_RUNNING;
  pl_ = coroutine<void>::pull_type(
    [this, fn = std::move(fn)](coroutine<void>::push_type& ps) mutable {
      // auto co = std::static_pointer_cast<AsioCoroutine>(shared_from_this());
      ps_ = &ps;
      fn(this);
      state_ = CoroutineState::COROUTINE_DEAD;
    });
  state_ = CoroutineState::COROUTINE_SUSPEND;
}

void
AsioCoroutine::yield()
{
  assert(ps_);
  assert(*ps_);
  assert(state_ == CoroutineState::COROUTINE_RUNNING);
  (*ps_)();
  assert(state_ == CoroutineState::COROUTINE_RUNNING);
}

void
AsioCoroutine::yield_for(int milli)
{
  assert(ps_);
  assert(*ps_);

  timer_.expires_from_now(std::chrono::milliseconds(milli));
  timer_.async_wait(
    [co = std::static_pointer_cast<AsioCoroutine>(shared_from_this())](
      boost::system::error_code ec) mutable {
      if (ec)
        return;

      if (co->pl_) {
        assert(co->state_ == CoroutineState::COROUTINE_SUSPEND);
        co->state_ = CoroutineState::COROUTINE_RUNNING;
        co->pl_();
        co->state_ = CoroutineState::COROUTINE_SUSPEND;
      }
    });

  assert(state_ == CoroutineState::COROUTINE_RUNNING);
  (*ps_)();
  assert(state_ == CoroutineState::COROUTINE_RUNNING);
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
      if (co->pl_) {
        assert(co->state_ == CoroutineState::COROUTINE_SUSPEND);
        co->state_ = CoroutineState::COROUTINE_RUNNING;
        co->pl_();
        co->state_ = CoroutineState::COROUTINE_SUSPEND;
      }
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
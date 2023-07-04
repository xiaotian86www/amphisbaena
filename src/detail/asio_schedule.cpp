#include "detail/asio_schedule.hpp"
#include "schedule.hpp"

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/coroutine/exceptions.hpp>
#include <boost/system/errc.hpp>
#include <functional>
#include <memory>
#include <unordered_map>

namespace translator {

void
Coroutine::yield()
{
  auto co = co_.lock().get();
  co->yield();
}

void
Coroutine::yield_for(int milli)
{
  auto co = co_.lock().get();
  co->yield_for(milli);
}

void
Coroutine::resume()
{
  if (auto co = co_.lock()) {
    co->resume();
  }
}

CoroutineImpl::CoroutineImpl(boost::asio::io_service& ios,
                             ScheduleRef sch,
                             task&& fn)
  : sch_(sch)
  , timer_(ios)
  , ps_([this, fn = std::move(fn)](coroutine<void>::pull_type& pl) mutable {
    pl_ = &pl;
    fn(sch_, weak_from_this());
    state_ = CoroutineState::COROUTINE_DEAD;
  })
  , pl_(nullptr)
{
}

CoroutineImpl::~CoroutineImpl() {}

void
CoroutineImpl::yield()
{
  do_yield();
}

void
CoroutineImpl::yield_for(int milli)
{
  timer_.expires_from_now(std::chrono::milliseconds(milli));
  timer_.async_wait(
    [co = std::static_pointer_cast<CoroutineImpl>(shared_from_this())](
      boost::system::error_code ec) mutable {
      if (ec)
        return;

      co->do_resume();
    });

  do_yield();
}

void
CoroutineImpl::resume()
{
  sch_.post([co = std::static_pointer_cast<CoroutineImpl>(shared_from_this())] {
    co->do_resume();
  });
}

void
CoroutineImpl::do_yield()
{
  assert(pl_);
  assert(*pl_);
  assert(state_ == CoroutineState::COROUTINE_RUNNING);
  (*pl_)();
  assert(state_ == CoroutineState::COROUTINE_RUNNING);
}

void
CoroutineImpl::do_resume()
{
  boost::system::error_code ec;
  timer_.cancel(ec);

  if (ps_) {
    assert(state_ == CoroutineState::COROUTINE_READY);
    state_ = CoroutineState::COROUTINE_RUNNING;
    ps_();
    if (state_ == CoroutineState::COROUTINE_DEAD)
      return;
    assert(state_ == CoroutineState::COROUTINE_RUNNING);
    state_ = CoroutineState::COROUTINE_READY;
  }
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
AsioSchedule::spawn(task&& fn)
{
  auto co =
    std::make_shared<CoroutineImpl>(ios_, weak_from_this(), std::move(fn));

  cos_.insert(co);

  co->resume();
}

void
AsioSchedule::post(std::function<void()>&& fn)
{
  ios_.post(std::move(fn));
}

void
AsioSchedule::resume(Coroutine co)
{
  ios_.post([co]() mutable { co.resume(); });
}

}
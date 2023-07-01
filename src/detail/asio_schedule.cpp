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
AsioCoroutine::set_func(task&& fn)
{
  ps_ = coroutine<void>::push_type(
    [co = std::static_pointer_cast<AsioCoroutine>(shared_from_this()),
     fn = std::move(fn)](coroutine<void>::pull_type& pl) mutable {
      co->pl_ = &pl;
      // state = CoroutineState::RUNNING;
      fn(co);
      // state = CoroutineState::DEAD;
    });
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

Schedule::Impl::Impl()
//   : acceptor_(ios_)
//   , awake_(std::bind(&Schedule2::do_accept, this, std::placeholders::_1))
{
  //   awake_(boost::system::errc::make_error_code(boost::system::errc::success));
}

void
Schedule::Impl::run()
{
  try {
    ios_.run();
  } catch (...) {
  }
  // cos_.clear();
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
    auto co = std::make_shared<AsioCoroutine>(shared_from_this());
    // cos_.insert(co);
    co->set_func(std::move(fn));
    co->resume();
  });
}

// void
// Schedule::Impl::yield()
// {
//   // assert(running_co_);
//   // running_co_->yield();
// }

// void
// Schedule::Impl::yield_for(int milli)
// {
//   // assert(running_co_);
//   // running_co_->yield_for(milli);
// }

// void
// Schedule::Impl::resume(std::weak_ptr<Coroutine> co)
// {
//   // ios_.post([this, co]() mutable {
//   //   auto sco = std::static_pointer_cast<AsioCoroutine>(co.lock());
//   //   if (sco) {
//   //     switch (sco->state) {
//   //       case CoroutineState::SUSPEND:
//   //         sco->state = CoroutineState::RUNNING;
//   //         do_resume(sco);
//   //         break;
//   //       case CoroutineState::DYING:
//   //         sco->state = CoroutineState::DEAD;
//   //       case CoroutineState::DEAD:
//   //         cos_.erase(sco);
//   //         break;
//   //       case CoroutineState::READY:
//   //       case CoroutineState::RUNNING:
//   //         assert(false);
//   //         break;
//   //     }
//   //   }
//   // });
// }

// void
// Schedule::Impl::do_resume(std::shared_ptr<Coroutine> co)
// {
//   assert(!running_co_);
//   assert(co);
//   running_co_ = co;

//   running_co_->resume();
//   switch (running_co_->state) {
//     case CoroutineState::RUNNING:
//       running_co_->state = CoroutineState::SUSPEND;
//       break;
//     case CoroutineState::DYING:
//       running_co_->state = CoroutineState::DEAD;
//     case CoroutineState::DEAD:
//       cos_.erase(running_co_);
//       break;
//     case CoroutineState::READY:
//     case CoroutineState::SUSPEND:
//       assert(false);
//       break;
//   }
//   running_co_.reset();
// }

}
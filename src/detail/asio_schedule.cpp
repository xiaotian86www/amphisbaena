#include "asio_schedule.hpp"

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/coroutine/exceptions.hpp>
#include <boost/system/errc.hpp>
#include <functional>
#include <unordered_map>
#include <utility>

namespace translator {

Schedule::Impl::Impl()
//   : acceptor_(ios_)
//   , awake_(std::bind(&Schedule2::do_accept, this, std::placeholders::_1))
{
  //   awake_(boost::system::errc::make_error_code(boost::system::errc::success));
}

void
Schedule::Impl::run()
{
  ios_.run();
}

void
Schedule::Impl::stop()
{
  ios_.stop();
}

void
Schedule::Impl::post(task&& fn)
{
  auto co = std::make_shared<AsioCoroutine>(
    ios_, [this, fn = std::move(fn)] { fn(ScheduleRef(shared_from_this())); });
  cos_.insert(co);
  ios_.post([this, co]() mutable {
    assert(!running_co_);
    running_co_ = co;
    running_co_->resume();
    running_co_.reset();
  });
}

void
Schedule::Impl::yield()
{
  assert(running_co_);
  assert(running_co_->yield);
  (*running_co_->yield)();
}

void
Schedule::Impl::yield_for(int milli)
{
  assert(running_co_);
  assert(running_co_->yield);
  std::weak_ptr<Schedule::Coroutine> co = running_co_;
  running_co_->timer.expires_from_now(std::chrono::milliseconds(milli));
  running_co_->timer.async_wait(
    [this, co](boost::system::error_code ec) mutable {
      if (!ec)
        resume(co);
    });
  (*running_co_->yield)();
}

void
Schedule::Impl::resume(std::weak_ptr<Schedule::Coroutine> co)
{
  ios_.post([this, co]() mutable {
    assert(!running_co_);
    running_co_ = std::static_pointer_cast<AsioCoroutine>(co.lock());
    if (running_co_) {
      if (running_co_->resume) {
        running_co_->resume();
      }
      running_co_.reset();
    }
  });
}

// void
// Schedule2::do_accept(await_context await)
// {
//   for (;;) {
//     auto sock = std::make_shared<stream_protocol::socket>(ios_);
//     acceptor_.async_accept(*sock,
//                            [this](boost::system::error_code ec) { awake_(ec);
//                            });

//     if (await())
//       break;

//     auto iter = awakes_.insert(std::make_pair(
//       sock,
//       coroutine<boost::system::error_code>::push_type(
//         std::bind(&Schedule2::do_read, this, sock, std::placeholders::_1))));
//     iter.first->second(
//       boost::system::errc::make_error_code(boost::system::errc::success));
//   }
// }

// void
// Schedule2::do_read(std::shared_ptr<stream_protocol::socket> sock,
//                    await_context await)
// {
//   std::array<char, 8192> data;
//   for (;;) {
//     std::size_t size;
//     sock->async_read_some(boost::asio::buffer(data, data.size()),
//                           [this, sock, &size](boost::system::error_code ec,
//                                               std::size_t in_size) mutable {
//                             size = in_size;
//                             auto iter = awakes_.find(sock);
//                             assert(iter != awakes_.end());
//                             iter->second(ec);
//                             if (iter->second.invalid())
//                               awakes_.erase(iter);
//                           });

//     if (await())
//       break;
//   }
// }

}
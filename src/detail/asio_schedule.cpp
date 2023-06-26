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

// void
// Schedule2::do_accept(await_context await)
// {
//   for (;;) {
//     auto sock = std::make_shared<stream_protocol::socket>(ios_);
//     acceptor_.async_accept(*sock,
//                            [this](boost::system::error_code ec) { awake_(ec); });

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
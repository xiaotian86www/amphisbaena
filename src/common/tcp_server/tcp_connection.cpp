
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
#include <unistd.h>

#include "future.hpp"
#include "log.hpp"
#include "schedule.hpp"
#include "tcp_connection.hpp"

namespace amphisbaena {
TcpConnection::TcpConnection(ScheduleRef sch,
                             CoroutineRef co,
                             MessageHandler& message_handler,
                             tcp::socket sock)
  : Connection(sch, co, message_handler)
  , sock_(std::move(sock))
{
}

void
TcpConnection::send(std::string_view data)
{
  std::size_t send_size = 0;
  for (;;) {
    std::size_t size = 0;
    boost::system::error_code ec;
    sock_.async_write_some(
      boost::asio::const_buffer(data.data() + send_size,
                                data.size() - send_size),
      [&size, &ec, sch = sch_, co = co_](boost::system::error_code in_ec,
                                         std::size_t in_size) mutable {
        ec = in_ec;
        size = in_size;
        sch.resume(co);
      });

    co_.yield();

    if (ec)
      throw ec;

    send_size += size;

    if (send_size == data.size())
      break;
  }
}

bool
TcpConnection::recv()
{
  boost::system::error_code ec;
  std::size_t size;

  sock_.async_read_some(
    boost::asio::buffer(data_),
    [&ec, &size, sch = sch_, co = co_](boost::system::error_code in_ec,
                                       std::size_t in_size) mutable {
      size = in_size;
      ec = in_ec;
      sch.resume(co);
    });

  co_.yield();

  if (ec) {
    close();
    return false;
  }

  LOG_DEBUG("Recv size: {}", size);
  do_recv({ data_.data(), size });
  return true;
}

void
TcpConnection::close()
{
  boost::system::error_code ec;
  sock_.close(ec);
}

}
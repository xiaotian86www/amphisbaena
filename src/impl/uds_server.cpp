#include "impl/uds_server.hpp"

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
#include <unistd.h>

#include "context.hpp"
#include "parser.hpp"
#include "schedule.hpp"

namespace translator {
UDSSocket::UDSSocket(ScheduleRef sch,
                     CoroutineRef co,
                     stream_protocol::socket sock)
  : Connection(sch, co)
  , sock_(std::move(sock))
{
}

void
UDSSocket::send(std::string_view data)
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

std::size_t
UDSSocket::recv(char* buffer, std::size_t buf_len)
{
  boost::system::error_code ec;
  std::size_t size;

  sock_.async_read_some(
    boost::asio::buffer(buffer, buf_len),
    [&ec, &size, sch = sch_, co = co_](boost::system::error_code in_ec,
                                       std::size_t in_size) mutable {
      size = in_size;
      ec = in_ec;
      sch.resume(co);
    });

  co_.yield();

  if (ec) {
    close();
    return -1;
  }

  return size;
}

void
UDSSocket::close()
{
  boost::system::error_code ec;
  sock_.close(ec);
}

UDSServer::UDSServer(boost::asio::io_service& ios,
                     std::shared_ptr<Schedule> sch,
                     std::string_view file)
  : ios_(ios)
  , sch_(sch)
  , endpoint_(file)
  , acceptor_(ios)
{
}

UDSServer::~UDSServer() {}

void
UDSServer::start()
{
  if (acceptor_.is_open())
    return;

  acceptor_.open(endpoint_.protocol());

  acceptor_.set_option(stream_protocol::acceptor::reuse_address(true));

  unlink(endpoint_.path().c_str());
  acceptor_.bind(endpoint_);

  acceptor_.listen();

  sch_->spawn(std::bind(
    &UDSServer::handle, this, std::placeholders::_1, std::placeholders::_2));
}

void
UDSServer::stop()
{
  boost::system::error_code ec;
  acceptor_.close(ec);
}

stream_protocol::socket
UDSServer::accept(ScheduleRef sch, CoroutineRef co)
{
  stream_protocol::socket sock(ios_);
  for (;;) {
    boost::system::error_code ec;
    acceptor_.async_accept(
      sock, [&ec, sch, co](boost::system::error_code in_ec) mutable {
        ec = in_ec;
        sch.resume(co);
      });

    co.yield();

    if (!ec)
      return std::move(sock);
  }
}

void
UDSServer::handle(ScheduleRef sch, CoroutineRef co)
{
  // 接受连接
  auto sock = accept(sch, co);

  // 开启新协程接受连接
  sch_->spawn(std::bind(
    &UDSServer::handle, this, std::placeholders::_1, std::placeholders::_2));

  // 接受消息
  std::shared_ptr<Connection> conn =
    std::make_shared<UDSSocket>(sch, co, std::move(sock));
  auto parser = Context::get_instance().parser_factory->create(sch, co, conn);
  std::array<char, 8192> data;
  for (;;) {
    boost::system::error_code ec;
    std::size_t size = conn->recv(data.data(), data.size());
    if (size == -1) {
      break;
    }

    parser->on_data({ data.data(), size });
  }
}

}
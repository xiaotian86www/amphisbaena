
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
#include <unistd.h>

#include "log.hpp"
#include "schedule.hpp"
#include "uds_server.hpp"

namespace translator {
UDSConnection::UDSConnection(ScheduleRef sch,
                             CoroutineRef co,
                             stream_protocol::socket sock)
  : Connection(sch, co)
  , sock_(std::move(sock))
{
}

void
UDSConnection::send(std::string_view data)
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
UDSConnection::recv(char* buffer, std::size_t buf_len)
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
UDSConnection::close()
{
  boost::system::error_code ec;
  sock_.close(ec);
}

UDSServer::UDSServer(boost::asio::io_service& ios,
                     std::shared_ptr<Schedule> sch,
                     const std::filesystem::path& file)
  : ios_(ios)
  , sch_(sch)
  , endpoint_(file)
  , acceptor_(ios)
{
  LOG_INFO("UDSServer create");
  acceptor_.open(endpoint_.protocol());

  acceptor_.set_option(stream_protocol::acceptor::reuse_address(true));

  unlink(endpoint_.path().c_str());
  acceptor_.bind(endpoint_);

  LOG_INFO("Listen: {}", file.string());
  acceptor_.listen();

  sch_->spawn(std::bind(
    &UDSServer::handle, this, std::placeholders::_1, std::placeholders::_2));
}

UDSServer::~UDSServer()
{
  LOG_INFO("UDSServer destroy");
  boost::system::error_code ec;
  acceptor_.close(ec);
  unlink(endpoint_.path().c_str());
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

    if (!ec) {
      LOG_INFO("Accept: {}", sock.remote_endpoint().path());
      return std::move(sock);
    } else {
      LOG_INFO("Accept failed. what: {}", ec.what());
    }
  }
}

void
UDSServer::handle(ScheduleRef sch, CoroutineRef co)
{
  // 接受连接
  auto sock = accept(sch, co);
  std::array<char, 8192> data;

  // 开启新协程接受连接
  sch_->spawn(std::bind(
    &UDSServer::handle, this, std::placeholders::_1, std::placeholders::_2));

  // 接受消息
  std::shared_ptr<UDSConnection> conn =
    std::make_shared<UDSConnection>(sch, co, std::move(sock));
  for (;;) {
    boost::system::error_code ec;
    std::size_t size = conn->recv(data.data(), data.size());
    LOG_DEBUG("Recv size: {}", size);
    
    if (size == -1) {
      break;
    }

    assert(message_handler);
    message_handler->on_recv(sch,
                             co,
                             std::static_pointer_cast<Connection>(conn),
                             { data.data(), size });
  }
}

}
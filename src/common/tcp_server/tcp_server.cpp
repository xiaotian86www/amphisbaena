
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
#include <unistd.h>

#include "log.hpp"
#include "schedule.hpp"
#include "tcp_server.hpp"

namespace amphisbaena {
TcpConnection::TcpConnection(ScheduleRef sch, CoroutineRef co, tcp::socket sock)
  : Connection(sch, co)
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

std::size_t
TcpConnection::recv(char* buffer, std::size_t buf_len)
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

  LOG_DEBUG("Recv size: {}", size);
  return size;
}

void
TcpConnection::close()
{
  boost::system::error_code ec;
  sock_.close(ec);
}

TcpServer::TcpServer(boost::asio::io_service& ios,
                     std::shared_ptr<Schedule> sch,
                     uint16_t port,
                     MessageHandler* message_handler)
  : Server(message_handler)
  , ios_(ios)
  , sch_(sch)
  , endpoint_(tcp::v4(), port)
  , acceptor_(ios_)
{
  LOG_INFO("TcpServer create");
  acceptor_.open(endpoint_.protocol());

  acceptor_.set_option(tcp::acceptor::reuse_address(true));

  acceptor_.bind(endpoint_);

  LOG_INFO("Listen: {}", port);
  acceptor_.listen();

  sch_->spawn(std::bind(
    &TcpServer::handle, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
  LOG_INFO("TcpServer destroy");
  boost::system::error_code ec;
  acceptor_.close(ec);
}

tcp::socket
TcpServer::accept(ScheduleRef sch, CoroutineRef co)
{
  tcp::socket sock(ios_);
  for (;;) {
    boost::system::error_code ec;
    acceptor_.async_accept(
      sock, [&ec, sch, co](boost::system::error_code in_ec) mutable {
        ec = in_ec;
        sch.resume(co);
      });

    co.yield();

    if (!ec) {
      LOG_INFO("Accept: {}", sock.remote_endpoint().address().to_string());
      return std::move(sock);
    } else {
      LOG_INFO("Accept failed. what: {}", ec.what());
    }
  }
}

void
TcpServer::handle(ScheduleRef sch, CoroutineRef co)
{
  // 接受连接
  auto sock = accept(sch, co);
  std::array<char, 8192> data;

  // 开启新协程接受连接
  sch_->spawn(std::bind(
    &TcpServer::handle, this, std::placeholders::_1, std::placeholders::_2));

  // 接受消息
  std::shared_ptr<TcpConnection> conn =
    std::make_shared<TcpConnection>(sch, co, std::move(sock));
  for (;;) {
    std::size_t size = conn->recv(data.data(), data.size());

    if (size == -1) {
      break;
    }

    assert(message_handler_);
    message_handler_->on_recv(sch,
                              co,
                              std::static_pointer_cast<Connection>(conn),
                              { data.data(), size });
  }
}

TcpServerFactory::TcpServerFactory(boost::asio::io_service& ios,
                                   std::shared_ptr<Schedule> sch,
                                   uint16_t port)
  : ios_(ios)
  , sch_(sch)
  , port_(port)
{
}

std::unique_ptr<Server>
TcpServerFactory::create(Server::MessageHandler* message_handler)
{
  return std::make_unique<TcpServer>(ios_, sch_, port_, message_handler);
}

}
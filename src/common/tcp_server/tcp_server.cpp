
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
#include "tcp_server.hpp"

namespace amphisbaena {
TcpServer::TcpServer(boost::asio::io_service& ios,
                     std::shared_ptr<Schedule> sch,
                     uint16_t port,
                     Connection::MessageHandler& message_handler)
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
      return sock;
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

  // 开启新协程接受连接
  sch_->spawn(std::bind(
    &TcpServer::handle, this, std::placeholders::_1, std::placeholders::_2));

  // 接受消息
  auto conn =
    std::make_shared<TcpConnection>(sch, co, message_handler_, std::move(sock));
  while (conn->recv()) {
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
TcpServerFactory::create(Connection::MessageHandler& message_handler)
{
  return std::make_unique<TcpServer>(ios_, sch_, port_, message_handler);
}

}
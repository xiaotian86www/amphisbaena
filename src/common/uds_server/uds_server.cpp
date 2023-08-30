
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
#include <unistd.h>

#include "log.hpp"
#include "schedule.hpp"
#include "uds_connection.hpp"
#include "uds_server.hpp"

namespace amphisbaena {
UdsServer::UdsServer(boost::asio::io_service& ios,
                     std::shared_ptr<Schedule> sch,
                     const std::filesystem::path& file,
                     Connection::MessageHandler& message_handler)
  : Server(message_handler)
  , ios_(ios)
  , sch_(sch)
  , endpoint_(file)
  , acceptor_(ios_)
{
  LOG_INFO("UdsServer create");
  acceptor_.open(endpoint_.protocol());

  acceptor_.set_option(stream_protocol::acceptor::reuse_address(true));

  unlink(endpoint_.path().c_str());
  acceptor_.bind(endpoint_);

  LOG_INFO("Listen: {}", file.string());
  acceptor_.listen();

  sch_->spawn(std::bind(
    &UdsServer::handle, this, std::placeholders::_1, std::placeholders::_2));
}

UdsServer::~UdsServer()
{
  LOG_INFO("UdsServer destroy");
  boost::system::error_code ec;
  acceptor_.close(ec);
  unlink(endpoint_.path().c_str());
}

stream_protocol::socket
UdsServer::accept(ScheduleRef sch, CoroutineRef co)
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
      return sock;
    } else {
      LOG_INFO("Accept failed. what: {}", ec.what());
    }
  }
}

void
UdsServer::handle(ScheduleRef sch, CoroutineRef co)
{
  // 接受连接
  auto sock = accept(sch, co);

  // 开启新协程接受连接
  sch_->spawn(std::bind(
    &UdsServer::handle, this, std::placeholders::_1, std::placeholders::_2));

  // 接受消息
  auto conn =
    std::make_shared<UdsConnection>(sch, co, message_handler_, std::move(sock));
  while (conn->recv()) {
  }
}

UdsServerFactory::UdsServerFactory(boost::asio::io_service& ios,
                                   std::shared_ptr<Schedule> sch,
                                   const std::filesystem::path& file)
  : ios_(ios)
  , sch_(sch)
  , file_(file)
{
}

std::unique_ptr<Server>
UdsServerFactory::create(Connection::MessageHandler& message_handler)
{
  return std::make_unique<UdsServer>(ios_, sch_, file_, message_handler);
}

}
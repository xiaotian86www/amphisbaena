#pragma once

#include <array>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <filesystem>
#include <memory>
#include <string_view>

#include "schedule.hpp"
#include "server.hpp"
#include "session.hpp"

using namespace boost::asio::ip;

namespace amphisbaena {
class TcpConnection
  : public std::enable_shared_from_this<Connection>
  , public Connection
{
public:
  TcpConnection(ScheduleRef sch, CoroutineRef co, tcp::socket sock);

public:
  void send(std::string_view data) override;

  std::size_t recv(char* buffer, std::size_t buf_len);

  void close() override;

private:
  tcp::socket sock_;
};

class TcpServer
  : public std::enable_shared_from_this<TcpServer>
  , public Server
{
public:
  TcpServer(boost::asio::io_service& ios,
            std::shared_ptr<Schedule> sch,
            uint16_t port,
            MessageHandler* message_handler);

  virtual ~TcpServer();

private:
  tcp::socket accept(ScheduleRef sch, CoroutineRef co);

  void handle(ScheduleRef sch, CoroutineRef co);

private:
  boost::asio::io_service& ios_;
  std::shared_ptr<Schedule> sch_;
  tcp::endpoint endpoint_;
  tcp::acceptor acceptor_;
};

class TcpServerFactory : public ServerFactory
{
public:
  TcpServerFactory(boost::asio::io_service& ios,
                   std::shared_ptr<Schedule> sch,
                   uint16_t port);

public:
  std::unique_ptr<Server> create(
    Server::MessageHandler* message_handler) override;

private:
  boost::asio::io_service& ios_;
  std::shared_ptr<Schedule> sch_;
  uint16_t port_;
};

}
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
class TcpServer
  : public Server
{
public:
  TcpServer(boost::asio::io_service& ios,
            std::shared_ptr<Schedule> sch,
            uint16_t port,
            Connection::MessageHandler& message_handler);

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
    Connection::MessageHandler& message_handler) override;

private:
  boost::asio::io_service& ios_;
  std::shared_ptr<Schedule> sch_;
  uint16_t port_;
};

}
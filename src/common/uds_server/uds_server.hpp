#pragma once

#include <array>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <filesystem>
#include <memory>
#include <string_view>

#include "schedule.hpp"
#include "server.hpp"
#include "session.hpp"

using namespace boost::asio::local;

namespace amphisbaena {
class UDSConnection
  : public std::enable_shared_from_this<Connection>
  , public Connection
{
public:
  UDSConnection(ScheduleRef sch, CoroutineRef co, stream_protocol::socket sock);

public:
  void send(std::string_view data) override;

  std::size_t recv(char* buffer, std::size_t buf_len);

  void close() override;

private:
  stream_protocol::socket sock_;
};

class UDSServer
  : public std::enable_shared_from_this<UDSServer>
  , public Server
{
public:
  UDSServer(boost::asio::io_service& ios,
            std::shared_ptr<Schedule> sch,
            const std::filesystem::path& file,
            MessageHandler* message_handler);

  virtual ~UDSServer();

private:
  stream_protocol::socket accept(ScheduleRef sch, CoroutineRef co);

  void handle(ScheduleRef sch, CoroutineRef co);

private:
  boost::asio::io_service& ios_;
  std::shared_ptr<Schedule> sch_;
  stream_protocol::endpoint endpoint_;
  stream_protocol::acceptor acceptor_;
};

class UDSServerFactory : public ServerFactory
{
public:
  UDSServerFactory(boost::asio::io_service& ios,
                   std::shared_ptr<Schedule> sch,
                   const std::filesystem::path& file);

public:
  std::unique_ptr<Server> create(
    Server::MessageHandler* message_handler) override;

private:
  boost::asio::io_service& ios_;
  std::shared_ptr<Schedule> sch_;
  std::filesystem::path file_;
};

}
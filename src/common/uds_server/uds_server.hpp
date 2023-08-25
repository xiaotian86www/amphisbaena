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
class UdsConnection
  : public Connection
{
public:
  UdsConnection(ScheduleRef sch,
                CoroutineRef co,
                MessageHandler* message_handler,
                stream_protocol::socket sock);

public:
  void send(std::string_view data) override;

  bool recv() override;

  void close() override;

private:
  stream_protocol::socket sock_;
  std::array<char, 1024> data_;
};

class UdsServer
  : public Server
{
public:
  UdsServer(boost::asio::io_service& ios,
            std::shared_ptr<Schedule> sch,
            const std::filesystem::path& file,
            Connection::MessageHandler* message_handler);

  virtual ~UdsServer();

private:
  stream_protocol::socket accept(ScheduleRef sch, CoroutineRef co);

  void handle(ScheduleRef sch, CoroutineRef co);

private:
  boost::asio::io_service& ios_;
  std::shared_ptr<Schedule> sch_;
  stream_protocol::endpoint endpoint_;
  stream_protocol::acceptor acceptor_;
};

class UdsServerFactory : public ServerFactory
{
public:
  UdsServerFactory(boost::asio::io_service& ios,
                   std::shared_ptr<Schedule> sch,
                   const std::filesystem::path& file);

public:
  std::unique_ptr<Server> create(
    Connection::MessageHandler* message_handler) override;

private:
  boost::asio::io_service& ios_;
  std::shared_ptr<Schedule> sch_;
  std::filesystem::path file_;
};

}
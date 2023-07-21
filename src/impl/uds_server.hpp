#pragma once

#include <array>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <memory>
#include <string_view>

#include "server.hpp"
#include "schedule.hpp"
#include "service.hpp"

using namespace boost::asio::local;

namespace translator {
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
            std::string_view file);

  virtual ~UDSServer();

public:
  void start() override;

  void stop() override;

private:
  stream_protocol::socket accept(ScheduleRef sch, CoroutineRef co);

  void handle(ScheduleRef sch, CoroutineRef co);

private:
  boost::asio::io_service& ios_;
  std::shared_ptr<Schedule> sch_;
  stream_protocol::endpoint endpoint_;
  stream_protocol::acceptor acceptor_;
};
}
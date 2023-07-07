#pragma once

#include "schedule.hpp"
#include "server.hpp"

#include <array>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <memory>
#include <string_view>

using namespace boost::asio::local;

namespace translator {
class UDSSocket : public Socket
{
public:
  UDSSocket(boost::asio::io_service& ios);

public:
  void send(ScheduleRef sch, CoroutineRef co, std::string_view data) override;

public:
  stream_protocol::socket& native() { return sock_; }

private:
  stream_protocol::socket sock_;
};

class UDSServer : public std::enable_shared_from_this<UDSServer>
{
public:
  UDSServer(std::shared_ptr<Schedule> sch,
            std::shared_ptr<ProtocolFactory> proto_factory,
            std::string_view file);
  virtual ~UDSServer();

public:
  void listen();

private:
  void do_accept(ScheduleRef sch, CoroutineRef co);

  void do_read(ScheduleRef sch,
               CoroutineRef co,
               std::shared_ptr<UDSSocket> sock);

private:
  std::shared_ptr<Schedule> sch_;
  std::shared_ptr<ProtocolFactory> proto_factory_;
  stream_protocol::endpoint endpoint_;
  stream_protocol::acceptor acceptor_;
};
}
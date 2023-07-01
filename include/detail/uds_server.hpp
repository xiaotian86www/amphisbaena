#pragma once

#include "detail/asio_schedule.hpp"
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
  UDSSocket(std::shared_ptr<AsioSchedule> sch);

public:
  void send(std::shared_ptr<Coroutine> co, std::string_view data);

public:
  stream_protocol::socket& native() { return sock_; }

private:
  stream_protocol::socket sock_;
};

class UDSServer : public std::enable_shared_from_this<UDSServer>
{
public:
  UDSServer(std::shared_ptr<AsioSchedule> sch, std::string_view file);
  virtual ~UDSServer();

public:
  void listen();

public:
  void set_server(std::shared_ptr<Protocol> protocol) { protocol_ = protocol; }

private:
  void do_accept(std::shared_ptr<Coroutine> sch);

  void do_read(std::shared_ptr<UDSSocket> sock,
               std::shared_ptr<Coroutine> sch);

private:
  std::shared_ptr<AsioSchedule> sch_;
  std::shared_ptr<Protocol> protocol_;
  stream_protocol::endpoint endpoint_;
  stream_protocol::acceptor acceptor_;
};
}
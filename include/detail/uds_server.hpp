#pragma once

#include "detail/asio_schedule.hpp"
#include "schedule.hpp"

#include <array>
#include <boost/asio/local/stream_protocol.hpp>
#include <memory>
#include <string_view>

using namespace boost::asio::local;

namespace translator {
class UDSServer : public std::enable_shared_from_this<UDSServer>
{
public:
  UDSServer(std::shared_ptr<AsioSchedule> sch, std::string_view file);
  virtual ~UDSServer();

public:
  void listen();

protected:
  virtual void on_data(std::shared_ptr<stream_protocol::socket> sock,
                       std::shared_ptr<Coroutine> co,
                       std::string_view data);

  void send(std::shared_ptr<stream_protocol::socket> sock,
            std::shared_ptr<Coroutine> co,
            std::string_view data);

private:
  void do_accept(std::shared_ptr<Coroutine> sch);

  void do_read(std::shared_ptr<stream_protocol::socket> sock,
               std::shared_ptr<Coroutine> sch);

private:
  std::shared_ptr<AsioSchedule> sch_;
  stream_protocol::endpoint ep_;
  stream_protocol::acceptor acceptor_;
};
}
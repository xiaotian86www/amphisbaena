#include "detail/uds_server.hpp"

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <memory>
#include <unistd.h>

namespace translator {
UDSSocket::UDSSocket(boost::asio::io_service& ios)
  : sock_(ios)
{
}

void
UDSSocket::send(ScheduleRef sch, CoroutineRef co, std::string_view data)
{
  std::size_t send_size = 0;
  for (;;) {
    std::size_t size = 0;
    boost::system::error_code ec;
    sock_.async_write_some(
      boost::asio::const_buffer(data.data() + send_size,
                                data.size() - send_size),
      [&size, &ec, sch, co](boost::system::error_code in_ec,
                            std::size_t in_size) mutable {
        ec = in_ec;
        size = in_size;
        sch.resume(co);
      });

    co.yield();

    if (ec)
      throw ec;

    send_size += size;

    if (send_size == data.size())
      break;
  }
}

UDSServer::UDSServer(boost::asio::io_service& ios,
                     std::shared_ptr<Schedule> sch,
                     std::shared_ptr<ParserFactory> proto_factory,
                     std::string_view file)
  : ios_(ios)
  , sch_(sch)
  , proto_factory_(proto_factory)
  , endpoint_(file)
  , acceptor_(ios)
{
}

UDSServer::~UDSServer() {}

void
UDSServer::listen()
{
  if (acceptor_.is_open())
    return;

  acceptor_.open(endpoint_.protocol());

  acceptor_.set_option(stream_protocol::acceptor::reuse_address(true));

  unlink(endpoint_.path().c_str());
  acceptor_.bind(endpoint_);

  acceptor_.listen();

  sch_->spawn(std::bind(
    &UDSServer::do_accept, this, std::placeholders::_1, std::placeholders::_2));
}

void
UDSServer::do_accept(ScheduleRef sch, CoroutineRef co)
{
  for (;;) {
    auto sock = std::make_shared<UDSSocket>(ios_);
    boost::system::error_code ec;
    acceptor_.async_accept(
      sock->native(), [&ec, sch, co](boost::system::error_code in_ec) mutable {
        ec = in_ec;
        sch.resume(co);
      });

    co.yield();

    if (ec)
      continue;

    sch_->spawn(std::bind(&UDSServer::do_read,
                          this,
                          std::placeholders::_1,
                          std::placeholders::_2,
                          sock));
  }
}

void
UDSServer::do_read(ScheduleRef sch,
                   CoroutineRef co,
                   std::shared_ptr<UDSSocket> sock)
{
  auto proto = proto_factory_->create();
  std::array<char, 8192> data;
  for (;;) {
    boost::system::error_code ec;
    std::size_t size;
    sock->native().async_read_some(
      boost::asio::buffer(data, data.size()),
      [&ec, &size, sch, co](boost::system::error_code in_ec,
                            std::size_t in_size) mutable {
        size = in_size;
        ec = in_ec;
        sch.resume(co);
      });

    co.yield();

    if (ec)
      throw ec;

    proto->on_data(sch, co, sock, { data.data(), size });
  }
}

}
#include "detail/uds_server.hpp"

#include <boost/asio/buffer.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <memory>
#include <unistd.h>

namespace translator {
UDSSocket::UDSSocket(std::shared_ptr<AsioSchedule> sch)
  : sock_(sch->io_service())
{
}

void
UDSSocket::send(Coroutine* co, std::string_view data)
{
  std::size_t send_size = 0;
  for (;;) {
    std::size_t size = 0;
    boost::system::error_code ec;
    sock_.async_write_some(
      boost::asio::const_buffer(data.data() + send_size,
                                data.size() - send_size),
      [&size, &ec, co = co->shared_from_this()](boost::system::error_code in_ec,
                                                std::size_t in_size) {
        ec = in_ec;
        size = in_size;
        co->resume();
      });

    co->yield();

    if (ec)
      throw ec;

    send_size += size;

    if (send_size == data.size())
      break;
  }
}

UDSServer::UDSServer(std::shared_ptr<AsioSchedule> sch,
                     std::shared_ptr<ProtocolFactory> proto_factory,
                     std::string_view file)
  : sch_(sch)
  , proto_factory_(proto_factory)
  , endpoint_(file)
  , acceptor_(sch_->io_service())
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

  sch_->post(std::bind(&UDSServer::do_accept, this, std::placeholders::_1));
}

void
UDSServer::do_accept(Coroutine* co)
{
  for (;;) {
    auto sock = std::make_shared<UDSSocket>(sch_);
    boost::system::error_code ec;
    acceptor_.async_accept(
      sock->native(),
      [&ec, co = co->shared_from_this()](boost::system::error_code in_ec) {
        ec = in_ec;
        co->resume();
      });

    co->yield();

    if (ec)
      continue;

    sch_->post(
      std::bind(&UDSServer::do_read, this, sock, std::placeholders::_1));
  }
}

void
UDSServer::do_read(std::shared_ptr<UDSSocket> sock, Coroutine* co)
{
  auto proto = proto_factory_->create();
  std::array<char, 8192> data;
  for (;;) {
    boost::system::error_code ec;
    std::size_t size;
    sock->native().async_read_some(
      boost::asio::buffer(data, data.size()),
      [&ec, &size, co = co->shared_from_this()](boost::system::error_code in_ec,
                                                std::size_t in_size) mutable {
        size = in_size;
        ec = in_ec;
        co->resume();
      });

    co->yield();

    if (ec)
      throw ec;

    proto->on_data(sock, co, { data.data(), size });
  }
}

}
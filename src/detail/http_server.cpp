#include "http_server.hpp"

#include <memory>

namespace translator {
HttpServer::HttpServer(std::shared_ptr<AsioSchedule> sch)
  : sch_(sch)
  , acceptor_(sch_->io_service())
{
}

void
HttpServer::do_accept(std::shared_ptr<Coroutine> co)
{
  for (;;) {
    auto sock =
      std::make_shared<stream_protocol::socket>(sch_->io_service());
    boost::system::error_code ec;
    acceptor_.async_accept(*sock,
                           [this, co, &ec](boost::system::error_code in_ec) {
                             ec = in_ec;
                             co->resume();
                           });

    co->yield();

    if (ec)
      break;

    sch_->post(
      std::bind(&HttpServer::do_read, this, sock, std::placeholders::_1));
  }
}

void
HttpServer::do_read(std::shared_ptr<stream_protocol::socket> sock, std::shared_ptr<Coroutine> co)
{
  std::array<char, 8192> data;
  for (;;) {
    boost::system::error_code ec;
    std::size_t size;
    sock->async_read_some(
      boost::asio::buffer(data, data.size()),
      [this, co, &ec, &size](boost::system::error_code in_ec,
                             std::size_t in_size) mutable {
        size = in_size;
        ec = in_ec;
        co->resume();
      });

    co->yield();

    if (ec)
      break;
  }
}

}
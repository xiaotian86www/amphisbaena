#include "http_server.hpp"

#include <memory>

namespace translator {
HttpServer::HttpServer(std::shared_ptr<Schedule> sch)
  : sch_(sch)
  , acceptor_(sch_->impl_->io_service())
{
}

void
HttpServer::do_accept(ScheduleRef)
{
  for (;;) {
    auto co = sch_->this_co();
    auto sock =
      std::make_shared<stream_protocol::socket>(sch_->impl_->io_service());
    boost::system::error_code ec;
    acceptor_.async_accept(*sock,
                           [this, co, &ec](boost::system::error_code in_ec) {
                             ec = in_ec;
                             sch_->resume(co);
                           });

    sch_->yield();

    if (ec)
      break;

    sch_->post(
      std::bind(&HttpServer::do_read, this, sock, std::placeholders::_1));
  }
}

void
HttpServer::do_read(std::shared_ptr<stream_protocol::socket> sock, ScheduleRef)
{
  std::array<char, 8192> data;
  for (;;) {
    auto co = sch_->this_co();
    boost::system::error_code ec;
    std::size_t size;
    sock->async_read_some(
      boost::asio::buffer(data, data.size()),
      [this, co, &size, &ec](boost::system::error_code in_ec,
                             std::size_t in_size) mutable {
        size = in_size;
        ec = in_ec;
        sch_->resume(co);
      });

    sch_->yield();

    if (ec)
      break;
  }
}

}
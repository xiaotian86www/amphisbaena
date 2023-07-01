#include "uds_server.hpp"

#include <memory>

namespace translator {
UDSServer::UDSServer(std::shared_ptr<AsioSchedule> sch)
  : sch_(sch)
  , acceptor_(sch_->io_service())
{
  sch_->post(std::bind(&UDSServer::do_accept, this, std::placeholders::_1));
}

void
UDSServer::on_data(const char* data, std::size_t data_len)
{
}

void
UDSServer::do_accept(std::shared_ptr<Coroutine> co)
{
  for (;;) {
    auto sock = std::make_shared<stream_protocol::socket>(sch_->io_service());
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
      std::bind(&UDSServer::do_read, this, sock, std::placeholders::_1));
  }
}

void
UDSServer::do_read(std::shared_ptr<stream_protocol::socket> sock,
                    std::shared_ptr<Coroutine> co)
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

    on_data(data.data(), size);
  }
}

}
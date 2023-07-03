#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <functional>
#include <memory>

#include "detail/asio_schedule.hpp"
#include "detail/uds_server.hpp"
#include "mock/mock_server.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(uds_server, on_data)
{
  auto sch = std::make_shared<translator::AsioSchedule>();
  auto proto_factory = std::make_shared<MockProtocolFactory>();
  auto server = std::make_shared<translator::UDSServer>(
    sch->io_service(), sch, proto_factory, "server.socket");

  boost::asio::io_service io_service;
  boost::asio::local::stream_protocol::socket sock(io_service);

  std::string_view data("1234567890");

  EXPECT_CALL(*proto_factory, create()).WillOnce(testing::Invoke([data] {
    auto protocol = std::make_unique<MockProtocol>();

    EXPECT_CALL(*protocol,
                on_data(testing::_, testing::_, testing::StrEq(data)))
      .WillOnce(
        testing::Invoke([](std::shared_ptr<translator::Socket> sock,
                           translator::Coroutine* co,
                           std::string_view data) { sock->send(co, data); }));
    return protocol;
  }));

  server->listen();

  std::thread th(std::bind(&translator::AsioSchedule::run, sch));

  sock.connect("server.socket");
  sock.write_some(boost::asio::buffer(data));

  char buffer[100];
  auto size = sock.read_some(boost::asio::buffer(buffer, 100));
  EXPECT_PRED2([&](auto left, auto right) { return left == right; },
               std::string_view(buffer, size),
               data);
  sch->stop();

  th.join();
}

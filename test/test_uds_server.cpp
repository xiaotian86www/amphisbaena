#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <functional>
#include <memory>

#include "detail/asio_schedule.hpp"
#include "detail/uds_server.hpp"
#include "mock/mock_uds_server.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(uds_server, on_data)
{
  auto sch = std::make_shared<translator::AsioSchedule>();
  auto server = std::make_shared<MockUDSServer>(sch, "server.socket");
  boost::asio::io_service io_service;
  boost::asio::local::stream_protocol::socket sock(io_service);

  std::string_view data("1234567890");

  server->listen();
  std::thread th(std::bind(&translator::AsioSchedule::run, sch.get()));

  EXPECT_CALL(*server, on_data(testing::_, testing::_, testing::StrEq(data)))
    .WillOnce(testing::Invoke(std::bind(&MockUDSServer::send,
                                        server.get(),
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        std::placeholders::_3)));

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

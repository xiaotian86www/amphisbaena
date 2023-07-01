#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
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

  EXPECT_CALL(*server, on_data(testing::StrEq(data)))
    .WillOnce(
      testing::Invoke(std::bind(&translator::AsioSchedule::stop, sch.get())));

  sock.connect("server.socket");
  sock.write_some(boost::asio::buffer(data));

  th.join();
}

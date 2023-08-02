#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "fixture/fixture_schedule.hpp"
#include "mock/mock_server.hpp"
#include "plugin/http_server/uds_server.hpp"
#include "schedule.hpp"

class UDSServer : public FixtureSchedule
{
public:
  UDSServer()
    : server(std::make_shared<amphisbaena::UDSServer>(ios,
                                                      sch,
                                                      "server.socket",
                                                      &message_handler))
  {
  }

protected:
  MockServer::MockMessageHandler message_handler;
  std::shared_ptr<amphisbaena::UDSServer> server;
};

TEST_F(UDSServer, on_recv)
{
  boost::asio::io_service io_service;
  boost::asio::local::stream_protocol::socket sock(io_service);

  std::string_view data("1234567890");

  EXPECT_CALL(message_handler,
              on_recv(testing::_, testing::_, testing::_, testing::_))
    .WillOnce(testing::Invoke([](amphisbaena::ScheduleRef sch,
                                 amphisbaena::CoroutineRef co,
                                 amphisbaena::ConnectionRef conn,
                                 std::string_view data) { conn.send(data); }));

  sock.connect("server.socket");
  sock.write_some(boost::asio::buffer(data));

  char buffer[100];
  auto size = sock.read_some(boost::asio::buffer(buffer, 100));
  EXPECT_PRED2([&](auto left, auto right) { return left == right; },
               std::string_view(buffer, size),
               data);

  stop();
}

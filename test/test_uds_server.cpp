#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <functional>
#include <memory>

#include "detail/uds_server.hpp"
#include "mock/mock_protocol.hpp"
#include "schedule.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class UDSServer : public testing::Test
{
public:
  virtual void SetUp()
  {
    sch = std::make_shared<translator::Schedule>(ios);
    proto_factory = std::make_shared<MockProtocolFactory>();
    server = std::make_shared<translator::UDSServer>(
      ios, sch, proto_factory, "server.socket");
  }

  virtual void TearDown() {}

protected:
  boost::asio::io_service ios;
  std::thread th;
  std::shared_ptr<translator::Schedule> sch;
  std::shared_ptr<MockProtocolFactory> proto_factory;
  std::shared_ptr<translator::UDSServer> server;
};

TEST_F(UDSServer, on_data)
{
  boost::asio::io_service io_service;
  boost::asio::local::stream_protocol::socket sock(io_service);

  std::string_view data("1234567890");

  EXPECT_CALL(*proto_factory, create()).WillOnce(testing::Invoke([data] {
    auto protocol = std::make_unique<MockProtocol>();

    EXPECT_CALL(
      *protocol,
      on_data(testing::_, testing::_, testing::_, testing::StrEq(data)))
      .WillOnce(testing::Invoke(
        [](translator::ScheduleRef sch,
           translator::CoroutineRef co,
           std::shared_ptr<translator::Socket> sock,
           std::string_view data) { sock->send(sch, co, data); }));
    return protocol;
  }));

  server->listen();

  th = std::thread([this] { ios.run(); });

  sock.connect("server.socket");
  sock.write_some(boost::asio::buffer(data));

  char buffer[100];
  auto size = sock.read_some(boost::asio::buffer(buffer, 100));
  EXPECT_PRED2([&](auto left, auto right) { return left == right; },
               std::string_view(buffer, size),
               data);
               
  ios.stop();

  th.join();
}

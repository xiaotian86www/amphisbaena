#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <functional>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <llhttp.h>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include "fixture_schedule.hpp"
#include "mock_server.hpp"
#include "plugin/http_server/uds_server.hpp"
#include "schedule.hpp"
#include "tool/http_client.hpp"

class UDSServer : public FixtureSchedule
{
public:
  UDSServer()
    : server(std::make_shared<amphisbaena::UDSServer>(ios,
                                                      sch,
                                                      "server.socket",
                                                      &message_handler))
    , client(std::make_shared<HttpClient>("server.socket"))
  {
  }

protected:
  MockServer::MockMessageHandler message_handler;
  std::shared_ptr<amphisbaena::UDSServer> server;
  std::shared_ptr<HttpClient> client;
};

TEST_F(UDSServer, on_recv_bak)
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

TEST_F(UDSServer, on_recv)
{
  std::promise<void> pms;

  EXPECT_CALL(message_handler,
              on_recv(testing::_,
                      testing::_,
                      testing::_,
                      "GET / HTTP/1.1\r\n"
                      "Content-Type: application/json; charset=utf-8\r\n"
                      "Content-Length: 26\r\n"
                      "\r\n"
                      "{\"SenderCompID\":\"CLIENT1\"}"))
    .WillOnce(testing::Invoke([](amphisbaena::ScheduleRef sch,
                                 amphisbaena::CoroutineRef co,
                                 amphisbaena::ConnectionRef conn,
                                 std::string_view data) {
      conn.send("HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json; charset=utf-8\r\n"
                "Content-Length: 26\r\n"
                "\r\n"
                "{\"TargetCompID\":\"CLIENT1\"}");
    }));

  rapidjson::Document response_body(rapidjson::Type::kObjectType);
  response_body.AddMember(
    "TargetCompID", "CLIENT1", response_body.GetAllocator());

  EXPECT_CALL(
    *client,
    on_recv(llhttp_status::HTTP_STATUS_OK, "{\"TargetCompID\":\"CLIENT1\"}"))
    .WillOnce(testing::Invoke(
      [&pms](llhttp_status_t, std::string_view) { pms.set_value(); }));

  client->send("1.1", "GET", "/", "{\"SenderCompID\":\"CLIENT1\"}");

  pms.get_future().wait_for(std::chrono::milliseconds(10));
  stop();
}

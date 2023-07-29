

#include <boost/asio/io_service.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include "builder.hpp"
#include "environment.hpp"
#include "fixture/fixture_schedule.hpp"
#include "impl/http_server.hpp"
#include "impl/json_message.hpp"
#include "message.hpp"
#include "mock/mock_builder.hpp"
#include "mock/mock_server.hpp"
#include "schedule.hpp"

class HttpServer : public FixtureSchedule
{
public:
  HttpServer()
    : server(new MockServer())
    , http_server(std::unique_ptr<MockServer>(server))
    , message_builder(std::make_shared<MockMessageBuilder>("GET /"))
  {
    translator::MessageBuilder::registe(
      { { message_builder->name(), message_builder } });
  }

  ~HttpServer() { translator::MessageBuilder::unregiste(); }

protected:
  MockServer* server;
  translator::HttpServer http_server;
  std::shared_ptr<MockMessageBuilder> message_builder;
};

TEST_F(HttpServer, on_data)
{
  EXPECT_CALL(
    *message_builder,
    create(testing::_, testing::Truly([](translator::MessagePtr message) {
           auto head = message->get_head();
           auto body = message->get_body();
           return head->get_string("method") == "GET" &&
                  head->get_string("url") == "/" &&
                  body->get_string("SenderCompID") == "CLIENT1";
         })))
    .Times(2)
    .WillRepeatedly(testing::Invoke([] {
      auto response = std::make_shared<translator::JsonMessage>();
      auto response_body = response->get_body();

      response_body->set_value("SenderCompID", "CLIENT1");

      return response;
    }));

  EXPECT_CALL(*server, start).Times(1);
  EXPECT_CALL(*server, stop).Times(1);

  http_server.start();

  sch->spawn([this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    auto conn = std::make_shared<MockConnection>(sch, co);

    EXPECT_CALL(
      *conn,
      send(testing::StrEq("HTTP/1.1 200 OK\r\n"
                          "Content-Type: application/json; charset=utf-8\r\n"
                          "Content-Length: 26\r\n"
                          "\r\n"
                          "{\"SenderCompID\":\"CLIENT1\"}")))
      .Times(2)
      .WillRepeatedly(testing::Return());

    http_server.on_recv(sch,
                        co,
                        conn,
                        "GET / HTTP/1.1\r\n"
                        "Content-Type: application/json; charset=utf-8\r\n"
                        "Content-Length: 27\r\n"
                        "\r\n"
                        "{\"SenderCompID\": \"CLIENT1\"}");
    http_server.on_recv(sch,
                        co,
                        conn,
                        "GET / HTTP/1.1\r\n"
                        "Content-Type: application/json; charset=utf-8\r\n"
                        "Content-Length: 27\r\n"
                        "\r\n"
                        "{\"SenderCompID\": \"CLIENT1\"}");
  });
}

TEST_F(HttpServer, on_data_not_found)
{
  EXPECT_CALL(*server, start).Times(1);
  EXPECT_CALL(*server, stop).Times(1);

  http_server.start();

  sch->spawn([this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    auto conn = std::make_shared<MockConnection>(sch, co);
    EXPECT_CALL(
      *conn,
      send(testing::StrEq("HTTP/1.1 404 NOT_FOUND\r\n"
                          "Content-Type: application/json; charset=utf-8\r\n"
                          "Content-Length: 2\r\n"
                          "\r\n"
                          "{}")))
      .Times(2)
      .WillRepeatedly(testing::Return());

    http_server.on_recv(sch, co, conn, "GET /root HTTP/1.1\r\n\r\n");
    http_server.on_recv(sch, co, conn, "GET /root HTTP/1.1\r\n\r\n");
  });
}

TEST_F(HttpServer, on_data_fail)
{
  EXPECT_CALL(
    *message_builder,
    create(testing::_, testing::Truly([](translator::MessagePtr message) {
           auto head = message->get_head();
           return head->get_value("method", "") == "GET" &&
                  head->get_value("url", "") == "/";
         })))
    .WillOnce(testing::Return(std::make_shared<translator::JsonMessage>()));

  EXPECT_CALL(*server, start).Times(1);
  EXPECT_CALL(*server, stop).Times(1);

  http_server.start();

  sch->spawn([this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    auto conn = std::make_shared<MockConnection>(sch, co);
    EXPECT_CALL(
      *conn,
      send(testing::StrEq("HTTP/1.1 200 OK\r\n"
                          "Content-Type: application/json; charset=utf-8\r\n"
                          "Content-Length: 2\r\n"
                          "\r\n"
                          "{}")))
      .WillOnce(testing::Return());

    http_server.on_recv(sch, co, conn, "GET HTTP/1.1\r\n\r\n");
    http_server.on_recv(sch, co, conn, "GET / HTTP/1.1\r\n\r\n");
  });
}
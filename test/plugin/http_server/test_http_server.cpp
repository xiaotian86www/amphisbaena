

#include <boost/asio/io_service.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include "builder.hpp"
#include "environment.hpp"
#include "fixture_schedule.hpp"
#include "message.hpp"
#include "mock_builder.hpp"
#include "mock_server.hpp"
#include "plugin/http_server/http_message.hpp"
#include "plugin/http_server/http_server.hpp"
#include "plugin/http_server/server.hpp"
#include "schedule.hpp"

class HttpServer : public FixtureSchedule
{
public:
  HttpServer()
    : http_message_factory(std::make_shared<amphisbaena::HttpMessageFactory>())
    , server_factory(std::make_shared<MockServerFactory>())
    , http_server(std::make_shared<amphisbaena::HttpServer>(server_factory))
    , message_builder(std::make_shared<MockMessageBuilder>("GET /"))
  {
    amphisbaena::MessageFactory::registe(http_message_factory);
    amphisbaena::MessageBuilder::registe(message_builder);
  }

  ~HttpServer()
  {
    amphisbaena::MessageFactory::unregiste();
    amphisbaena::MessageBuilder::unregiste();
  }

protected:
  std::shared_ptr<amphisbaena::MessageFactory> http_message_factory;
  std::shared_ptr<amphisbaena::ServerFactory> server_factory;
  std::shared_ptr<amphisbaena::HttpServer> http_server;
  std::shared_ptr<MockMessageBuilder> message_builder;
};

TEST_F(HttpServer, on_data)
{
  EXPECT_CALL(
    *message_builder,
    create(testing::_, testing::Truly([](amphisbaena::MessagePtr message) {
             auto head = message->get_head();
             auto body = message->get_body();
             return head->get_string("method") == "GET" &&
                    head->get_string("url") == "/" &&
                    body->get_string("SenderCompID") == "CLIENT1";
           })))
    .Times(2)
    .WillRepeatedly(testing::Invoke([] {
      auto response = std::make_shared<amphisbaena::HttpMessage>();
      auto response_body = response->get_body();

      response_body->set_value("SenderCompID", "CLIENT1");

      return response;
    }));

  sch->spawn(
    [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
      auto conn = std::make_shared<MockConnection>(sch, co);

      EXPECT_CALL(
        *conn,
        send("HTTP/1.1 200 OK\r\n"
                            "Content-Type: application/json; charset=utf-8\r\n"
                            "Content-Length: 26\r\n"
                            "\r\n"
                            "{\"SenderCompID\":\"CLIENT1\"}"))
        .Times(2)
        .WillRepeatedly(testing::Return());

      http_server->on_recv(sch,
                           co,
                           conn,
                           "GET / HTTP/1.1\r\n"
                           "Content-Type: application/json; charset=utf-8\r\n"
                           "Content-Length: 27\r\n"
                           "\r\n"
                           "{\"SenderCompID\": \"CLIENT1\"}");
      http_server->on_recv(sch,
                           co,
                           conn,
                           "GET / HTTP/1.1\r\n"
                           "Content-Type: application/json; charset=utf-8\r\n"
                           "Content-Length: 27\r\n"
                           "\r\n"
                           "{\"SenderCompID\": \"CLIENT1\"}");
    });
}

TEST_F(HttpServer, on_data_split)
{
  EXPECT_CALL(
    *message_builder,
    create(testing::_, testing::Truly([](amphisbaena::MessagePtr message) {
             auto head = message->get_head();
             auto body = message->get_body();
             return head->get_string("method") == "GET" &&
                    head->get_string("url") == "/" &&
                    body->get_string("SenderCompID") == "CLIENT1";
           })))
    .Times(2)
    .WillRepeatedly(testing::Invoke([] {
      auto response = std::make_shared<amphisbaena::HttpMessage>();
      auto response_body = response->get_body();

      response_body->set_value("SenderCompID", "CLIENT1");

      return response;
    }));

  sch->spawn(
    [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
      auto conn = std::make_shared<MockConnection>(sch, co);

      EXPECT_CALL(
        *conn,
        send("HTTP/1.1 200 OK\r\n"
                            "Content-Type: application/json; charset=utf-8\r\n"
                            "Content-Length: 26\r\n"
                            "\r\n"
                            "{\"SenderCompID\":\"CLIENT1\"}"))
        .Times(2)
        .WillRepeatedly(testing::Return());

      http_server->on_recv(sch,
                           co,
                           conn,
                           "GET / HTTP/1.1\r\n"
                           "Content-Type: application/json; charset=utf-8\r\n"
                           "Content-Length: 27\r\n"
                           "\r\n"
                           "{\"SenderCompID\": ");
      http_server->on_recv(sch, co, conn, "\"CLIENT1\"}");
      http_server->on_recv(sch,
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
  sch->spawn(
    [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
      auto conn = std::make_shared<MockConnection>(sch, co);
      EXPECT_CALL(
        *conn,
        send("HTTP/1.1 404 NOT_FOUND\r\n"
                            "Content-Type: application/json; charset=utf-8\r\n"
                            "Content-Length: 37\r\n"
                            "\r\n"
                            "{\"description\":\"GET /root not found\"}"))
        .Times(2)
        .WillRepeatedly(testing::Return());

      http_server->on_recv(sch, co, conn, "GET /root HTTP/1.1\r\n\r\n");
      http_server->on_recv(sch, co, conn, "GET /root HTTP/1.1\r\n\r\n");
    });
}

TEST_F(HttpServer, on_data_fail)
{
  EXPECT_CALL(
    *message_builder,
    create(testing::_, testing::Truly([](amphisbaena::MessagePtr message) {
             auto head = message->get_head();
             return head->get_value("method", "") == "GET" &&
                    head->get_value("url", "") == "/";
           })))
    .WillOnce(testing::Return(std::make_shared<amphisbaena::HttpMessage>()));

  sch->spawn(
    [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
      auto conn = std::make_shared<MockConnection>(sch, co);
      EXPECT_CALL(
        *conn,
        send("HTTP/1.1 200 OK\r\n"
                            "Content-Type: application/json; charset=utf-8\r\n"
                            "Content-Length: 2\r\n"
                            "\r\n"
                            "{}"))
        .WillOnce(testing::Return());

      http_server->on_recv(sch, co, conn, "GET HTTP/1.1\r\n\r\n");
      http_server->on_recv(sch, co, conn, "GET / HTTP/1.1\r\n\r\n");
    });
}


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
    , message_builder(std::make_shared<translator::MessageBuilder>())
  {
    http_server.message_builder = message_builder;
    message_builder->registe("GET /", message_ctor.AsStdFunction());
  }

protected:
  MockServer* server;
  translator::HttpServer http_server;
  std::shared_ptr<translator::MessageBuilder> message_builder;
  testing::MockFunction<translator::MessageBuilder::ctor_prototype>
    message_ctor;
};

TEST_F(HttpServer, on_data)
{
  EXPECT_CALL(
    message_ctor,
    Call(testing::_, testing::Truly([](translator::MessagePtr message) {
           const auto& body = message->get_body();
           return body.get_value("method", "") == "GET" &&
                  body.get_value("url", "") == "/";
         })))
    .Times(2)
    .WillRepeatedly(testing::Invoke(
      [] { return std::make_shared<translator::JsonMessage>(); }));

  EXPECT_CALL(*server, start).Times(1);
  EXPECT_CALL(*server, stop).Times(1);

  http_server.start();

  sch->spawn([this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    auto conn = std::make_shared<MockConnection>(sch, co);

    EXPECT_CALL(*conn,
                send(testing::StrEq(
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: application/json; charset=utf-8\r\n\r\n")))
      .Times(2)
      .WillRepeatedly(testing::Return());

    http_server.on_recv(sch, co, conn, "GET / HTTP/1.1\r\n\r\n");
    http_server.on_recv(sch, co, conn, "GET / HTTP/1.1\r\n\r\n");
  });
}

TEST_F(HttpServer, on_data_not_found)
{
  EXPECT_CALL(*server, start).Times(1);
  EXPECT_CALL(*server, stop).Times(1);

  http_server.start();

  sch->spawn([this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    auto conn = std::make_shared<MockConnection>(sch, co);
    EXPECT_CALL(*conn, send(testing::StrEq("HTTP/1.1 404 NOT_FOUND\r\n\r\n")))
      .Times(2)
      .WillRepeatedly(testing::Return());

    http_server.on_recv(sch, co, conn, "GET /root HTTP/1.1\r\n\r\n");
    http_server.on_recv(sch, co, conn, "GET /root HTTP/1.1\r\n\r\n");
  });
}

TEST_F(HttpServer, on_data_fail)
{
  EXPECT_CALL(
    message_ctor,
    Call(testing::_, testing::Truly([](translator::MessagePtr message) {
           const auto& body = message->get_body();
           return body.get_value("method", "") == "GET" &&
                  body.get_value("url", "") == "/";
         })))
    .WillOnce(testing::Return(std::make_shared<translator::JsonMessage>()));

  EXPECT_CALL(*server, start).Times(1);
  EXPECT_CALL(*server, stop).Times(1);

  http_server.start();

  sch->spawn([this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    auto conn = std::make_shared<MockConnection>(sch, co);
    EXPECT_CALL(*conn,
                send(testing::StrEq(
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: application/json; charset=utf-8\r\n\r\n")))
      .WillOnce(testing::Return());

    http_server.on_recv(sch, co, conn, "GET HTTP/1.1\r\n\r\n");
    http_server.on_recv(sch, co, conn, "GET / HTTP/1.1\r\n\r\n");
  });
}
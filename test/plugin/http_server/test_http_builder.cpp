

#include <boost/asio/io_service.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include "builder.hpp"
#include "common/http_message/http_message.hpp"
#include "common/http_parser/http_parser.hpp"
#include "environment.hpp"
#include "fixture_schedule.hpp"
#include "message.hpp"
#include "mock_builder.hpp"
#include "mock_server.hpp"
#include "mock_session.hpp"
#include "plugin/http_server/http_builder.hpp"
#include "schedule.hpp"

class HttpBuilder : public FixtureSchedule<testing::Test>
{
public:
  HttpBuilder()
    : http_message_factory(std::make_shared<amphisbaena::HttpMessageFactory>())
    , message_builder(std::make_shared<MockMessageBuilder>("GET /"))
    , server_factory(std::make_shared<MockServerFactory>())
    , http_builder(std::make_shared<amphisbaena::HttpBuilder>(server_factory))
  {
    amphisbaena::MessageFactory::registe(http_message_factory);
    amphisbaena::MessageBuilder::registe(message_builder);
  }

  ~HttpBuilder() { amphisbaena::MessageFactory::unregiste(); }

protected:
  std::shared_ptr<amphisbaena::MessageFactory> http_message_factory;
  std::shared_ptr<MockMessageBuilder> message_builder;
  std::shared_ptr<MockServerFactory> server_factory;
  std::shared_ptr<amphisbaena::HttpBuilder> http_builder;
};

TEST_F(HttpBuilder, on_data)
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
      auto conn =
        std::make_shared<MockConnection>(sch, co, http_builder->http_server());

      EXPECT_CALL(*conn,
                  send("HTTP/1.1 200 OK\r\n"
                       "Content-Type: application/json; charset=utf-8\r\n"
                       "Content-Length: 26\r\n"
                       "\r\n"
                       "{\"SenderCompID\":\"CLIENT1\"}"))
        .Times(2)
        .WillRepeatedly(testing::Return());

      conn->do_recv("GET / HTTP/1.1\r\n"
                 "Content-Type: application/json; charset=utf-8\r\n"
                 "Content-Length: 27\r\n"
                 "\r\n"
                 "{\"SenderCompID\": \"CLIENT1\"}");

      conn->do_recv("GET / HTTP/1.1\r\n"
                 "Content-Type: application/json; charset=utf-8\r\n"
                 "Content-Length: 27\r\n"
                 "\r\n"
                 "{\"SenderCompID\": \"CLIENT1\"}");
    });
}

TEST_F(HttpBuilder, on_data_split)
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
      auto conn =
        std::make_shared<MockConnection>(sch, co, http_builder->http_server());

      EXPECT_CALL(*conn,
                  send("HTTP/1.1 200 OK\r\n"
                       "Content-Type: application/json; charset=utf-8\r\n"
                       "Content-Length: 26\r\n"
                       "\r\n"
                       "{\"SenderCompID\":\"CLIENT1\"}"))
        .Times(2)
        .WillRepeatedly(testing::Return());

      conn->do_recv("GET / HTTP/1.1\r\n"
                 "Content-Type: application/json; charset=utf-8\r\n"
                 "Content-Length: 27\r\n"
                 "\r\n"
                 "{\"SenderCompID\": ");
      conn->do_recv("\"CLIENT1\"}");
      conn->do_recv("GET / HTTP/1.1\r\n"
                 "Content-Type: application/json; charset=utf-8\r\n"
                 "Content-Length: 27\r\n"
                 "\r\n"
                 "{\"SenderCompID\": \"CLIENT1\"}");
    });
}

TEST_F(HttpBuilder, on_data_not_found)
{
  sch->spawn(
    [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
      auto conn =
        std::make_shared<MockConnection>(sch, co, http_builder->http_server());
      EXPECT_CALL(*conn,
                  send("HTTP/1.1 404 NOT_FOUND\r\n"
                       "Content-Type: application/json; charset=utf-8\r\n"
                       "Content-Length: 37\r\n"
                       "\r\n"
                       "{\"description\":\"GET /root not found\"}"))
        .Times(2)
        .WillRepeatedly(testing::Return());

      conn->do_recv("GET /root HTTP/1.1\r\n\r\n");
      conn->do_recv("GET /root HTTP/1.1\r\n\r\n");
    });
}

TEST_F(HttpBuilder, on_data_fail)
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
      auto conn =
        std::make_shared<MockConnection>(sch, co, http_builder->http_server());
      EXPECT_CALL(*conn,
                  send("HTTP/1.1 200 OK\r\n"
                       "Content-Type: application/json; charset=utf-8\r\n"
                       "Content-Length: 2\r\n"
                       "\r\n"
                       "{}"))
        .WillOnce(testing::Return());

      conn->do_recv("GET HTTP/1.1\r\n\r\n");
      conn->do_recv("GET / HTTP/1.1\r\n\r\n");
    });
}
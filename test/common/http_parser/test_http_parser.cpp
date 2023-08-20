

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
#include "schedule.hpp"

class HttpParser : public FixtureSchedule<testing::Test>
{
public:
  HttpParser()
    : http_message_factory(std::make_shared<amphisbaena::HttpMessageFactory>())
    , server_factory(std::make_shared<MockServerFactory>())
    , message_handler(std::make_shared<MockSession::MessageHandler>())
    , http_server(
        std::make_shared<amphisbaena::HttpServer>(server_factory,
                                                  message_handler.get()))
  {
    amphisbaena::MessageFactory::registe(http_message_factory);
  }

  ~HttpParser() { amphisbaena::MessageFactory::unregiste(); }

protected:
  std::shared_ptr<amphisbaena::MessageFactory> http_message_factory;
  std::shared_ptr<amphisbaena::ServerFactory> server_factory;
  std::shared_ptr<MockSession::MessageHandler> message_handler;
  std::shared_ptr<amphisbaena::HttpServer> http_server;
};

TEST_F(HttpParser, on_recv)
{
  EXPECT_CALL(*message_handler,
              on_recv(testing::_,
                      testing::_,
                      testing::_,
                      testing::Truly([](amphisbaena::MessagePtr message) {
                        auto head = message->get_head();
                        auto body = message->get_body();
                        return head->get_string("method") == "GET" &&
                               head->get_string("url") == "/" &&
                               head->get_string("version") == "1.1" &&
                               body->get_string("SenderCompID") == "CLIENT1";
                      })))
    .Times(2)
    .WillRepeatedly(testing::Invoke([](amphisbaena::ScheduleRef /* sch */,
                                       amphisbaena::CoroutineRef /* co */,
                                       amphisbaena::SessionPtr session,
                                       amphisbaena::MessagePtr /* message */) {
      auto response = std::make_shared<amphisbaena::HttpMessage>();
      auto response_head = response->get_head();
      auto response_body = response->get_body();

      response_head->set_value("code", HTTP_STATUS_OK);
      response_head->set_value("version", "1.1");
      response_body->set_value("SenderCompID", "CLIENT1");

      session->send(response);
    }));

  sch->spawn(
    [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
      auto conn = std::make_shared<MockConnection>(sch, co);

      EXPECT_CALL(*conn,
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

// TEST_F(HttpServer, on_data)
// {
//   EXPECT_CALL(
//     *message_builder,
//     create(testing::_, testing::Truly([](amphisbaena::MessagePtr message) {
//              auto head = message->get_head();
//              auto body = message->get_body();
//              return head->get_string("method") == "GET" &&
//                     head->get_string("url") == "/" &&
//                     body->get_string("SenderCompID") == "CLIENT1";
//            })))
//     .Times(2)
//     .WillRepeatedly(testing::Invoke([] {
//       auto response = std::make_shared<amphisbaena::HttpMessage>();
//       auto response_body = response->get_body();

//       response_body->set_value("SenderCompID", "CLIENT1");

//       return response;
//     }));

//   sch->spawn(
//     [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
//       auto conn = std::make_shared<MockConnection>(sch, co);

//       EXPECT_CALL(*conn,
//                   send("HTTP/1.1 200 OK\r\n"
//                        "Content-Type: application/json; charset=utf-8\r\n"
//                        "Content-Length: 26\r\n"
//                        "\r\n"
//                        "{\"SenderCompID\":\"CLIENT1\"}"))
//         .Times(2)
//         .WillRepeatedly(testing::Return());

//       http_server->on_recv(sch,
//                            co,
//                            conn,
//                            "GET / HTTP/1.1\r\n"
//                            "Content-Type: application/json;
//                            charset=utf-8\r\n" "Content-Length: 27\r\n"
//                            "\r\n"
//                            "{\"SenderCompID\": \"CLIENT1\"}");
//       http_server->on_recv(sch,
//                            co,
//                            conn,
//                            "GET / HTTP/1.1\r\n"
//                            "Content-Type: application/json;
//                            charset=utf-8\r\n" "Content-Length: 27\r\n"
//                            "\r\n"
//                            "{\"SenderCompID\": \"CLIENT1\"}");
//     });
// }

// TEST_F(HttpServer, on_data_split)
// {
//   EXPECT_CALL(
//     *message_builder,
//     create(testing::_, testing::Truly([](amphisbaena::MessagePtr message) {
//              auto head = message->get_head();
//              auto body = message->get_body();
//              return head->get_string("method") == "GET" &&
//                     head->get_string("url") == "/" &&
//                     body->get_string("SenderCompID") == "CLIENT1";
//            })))
//     .Times(2)
//     .WillRepeatedly(testing::Invoke([] {
//       auto response = std::make_shared<amphisbaena::HttpMessage>();
//       auto response_body = response->get_body();

//       response_body->set_value("SenderCompID", "CLIENT1");

//       return response;
//     }));

//   sch->spawn(
//     [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
//       auto conn = std::make_shared<MockConnection>(sch, co);

//       EXPECT_CALL(*conn,
//                   send("HTTP/1.1 200 OK\r\n"
//                        "Content-Type: application/json; charset=utf-8\r\n"
//                        "Content-Length: 26\r\n"
//                        "\r\n"
//                        "{\"SenderCompID\":\"CLIENT1\"}"))
//         .Times(2)
//         .WillRepeatedly(testing::Return());

//       http_server->on_recv(sch,
//                            co,
//                            conn,
//                            "GET / HTTP/1.1\r\n"
//                            "Content-Type: application/json;
//                            charset=utf-8\r\n" "Content-Length: 27\r\n"
//                            "\r\n"
//                            "{\"SenderCompID\": ");
//       http_server->on_recv(sch, co, conn, "\"CLIENT1\"}");
//       http_server->on_recv(sch,
//                            co,
//                            conn,
//                            "GET / HTTP/1.1\r\n"
//                            "Content-Type: application/json;
//                            charset=utf-8\r\n" "Content-Length: 27\r\n"
//                            "\r\n"
//                            "{\"SenderCompID\": \"CLIENT1\"}");
//     });
// }

// TEST_F(HttpServer, on_data_not_found)
// {
//   sch->spawn(
//     [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
//       auto conn = std::make_shared<MockConnection>(sch, co);
//       EXPECT_CALL(*conn,
//                   send("HTTP/1.1 404 NOT_FOUND\r\n"
//                        "Content-Type: application/json; charset=utf-8\r\n"
//                        "Content-Length: 37\r\n"
//                        "\r\n"
//                        "{\"description\":\"GET /root not found\"}"))
//         .Times(2)
//         .WillRepeatedly(testing::Return());

//       http_server->on_recv(sch, co, conn, "GET /root HTTP/1.1\r\n\r\n");
//       http_server->on_recv(sch, co, conn, "GET /root HTTP/1.1\r\n\r\n");
//     });
// }

// TEST_F(HttpServer, on_data_fail)
// {
//   EXPECT_CALL(
//     *message_builder,
//     create(testing::_, testing::Truly([](amphisbaena::MessagePtr message) {
//              auto head = message->get_head();
//              return head->get_value("method", "") == "GET" &&
//                     head->get_value("url", "") == "/";
//            })))
//     .WillOnce(testing::Return(std::make_shared<amphisbaena::HttpMessage>()));

//   sch->spawn(
//     [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
//       auto conn = std::make_shared<MockConnection>(sch, co);
//       EXPECT_CALL(*conn,
//                   send("HTTP/1.1 200 OK\r\n"
//                        "Content-Type: application/json; charset=utf-8\r\n"
//                        "Content-Length: 2\r\n"
//                        "\r\n"
//                        "{}"))
//         .WillOnce(testing::Return());

//       http_server->on_recv(sch, co, conn, "GET HTTP/1.1\r\n\r\n");
//       http_server->on_recv(sch, co, conn, "GET / HTTP/1.1\r\n\r\n");
//     });
// }
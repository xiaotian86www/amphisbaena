#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "common/http_message/http_message.hpp"
#include "common/http_monitor/http_monitor.hpp"
#include "fixture_schedule.hpp"
#include "mock_connection.hpp"
#include "mock_server.hpp"

class HttpMonitor : public FixtureSchedule<testing::Test>
{
public:
  HttpMonitor()
    : http_message_factory(std::make_shared<amphisbaena::HttpMessageFactory>())
    , server_factory(std::make_shared<MockServerFactory>())
    , http_monitor(std::make_shared<amphisbaena::HttpMonitor>(server_factory))
  {
    amphisbaena::MessageFactory::registe(http_message_factory);
  }

  ~HttpMonitor() { amphisbaena::MessageFactory::unregiste(); }

protected:
  std::shared_ptr<amphisbaena::MessageFactory> http_message_factory;
  std::shared_ptr<MockServerFactory> server_factory;
  std::shared_ptr<amphisbaena::HttpMonitor> http_monitor;
};

TEST_F(HttpMonitor, load)
{
  sch->spawn(
    [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
      auto conn =
        std::make_shared<MockConnection>(sch, co, http_monitor->http_server());

      EXPECT_CALL(
        *conn,
        send("HTTP/1.1 200 OK\r\n"
             "Content-Type: application/json; charset=utf-8\r\n"
             "Content-Length: 72\r\n"
             "\r\n"
             "{\"code\":0,\"status\":200,\"message\":\"请求成功\",\"body\":{\"id\":\"plugin1\"}}"))
        .WillOnce(testing::Return());

      conn->do_recv("POST /v1/api/plugins/plugin1 HTTP/1.1\r\n"
                    "Content-Type: application/json; charset=utf-8\r\n"
                    "Content-Length: 37\r\n"
                    "\r\n"
                    "{\"path\": \"mock/plugin/libplugin1.so\"}");
    });
}

TEST_F(HttpMonitor, load_failed)
{
  sch->spawn(
    [this](amphisbaena::ScheduleRef sch, amphisbaena::CoroutineRef co) {
      auto conn =
        std::make_shared<MockConnection>(sch, co, http_monitor->http_server());

      EXPECT_CALL(
        *conn,
        send("HTTP/1.1 500 INTERNAL_SERVER_ERROR\r\n"
             "Content-Type: application/json; charset=utf-8\r\n"
             "Content-Length: 58\r\n"
             "\r\n"
             "{\"code\":0,\"status\":500,\"message\":\"内部错误\",\"body\":{}}"))
        .WillOnce(testing::Return());

      conn->do_recv("POST /v1/api/plugins/plugin1 HTTP/1.1\r\n"
                    "Content-Type: application/json; charset=utf-8\r\n"
                    "Content-Length: 37\r\n"
                    "\r\n"
                    "{\"path\": \"mock/plugin/libplugin2.so\"}");
    });
}
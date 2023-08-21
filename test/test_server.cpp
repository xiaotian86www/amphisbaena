#include <functional>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <llhttp.h>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

#include "fixture_server.hpp"
#include "schedule.hpp"

TEST_P(Server, on_recv)
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
    .WillOnce(testing::Invoke([](amphisbaena::ScheduleRef /* sch */,
                                 amphisbaena::CoroutineRef /* co */,
                                 amphisbaena::ConnectionRef conn,
                                 std::string_view /* data */) {
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
    on_recv(200, "{\"TargetCompID\":\"CLIENT1\"}"))
    .WillOnce(testing::Invoke(
      [&pms](uint16_t, std::string_view) { pms.set_value(); }));

  client->send("1.1", "GET", "/", "{\"SenderCompID\":\"CLIENT1\"}");

  pms.get_future().wait_for(std::chrono::milliseconds(10));
  stop();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(Server);

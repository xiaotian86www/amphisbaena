
#include "gmock/gmock.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include "builder.hpp"
#include "context.hpp"
#include "detail/http_parser.hpp"
#include "detail/json_object.hpp"
#include "environment.hpp"
#include "mock/mock_builder.hpp"
#include "mock/mock_parser.hpp"
#include "object.hpp"
#include "parser.hpp"
#include "schedule.hpp"

class Parser : public testing::Test
{
public:
  virtual void SetUp()
  {
    sch = std::make_shared<translator::Schedule>(ios);
    parser_factory = std::make_shared<translator::HttpParserFactory>();
    object_builder = std::make_shared<translator::ObjectBuilder>();

    translator::Context::get_instance().object_builder = object_builder;

    object_builder->registe("GET /", object_ctor.AsStdFunction());
  }

  virtual void TearDown() {}

protected:
  boost::asio::io_service ios;
  std::thread th;
  std::shared_ptr<translator::Schedule> sch;
  std::shared_ptr<translator::ParserFactory> parser_factory;
  std::shared_ptr<translator::ObjectBuilder> object_builder;
  testing::MockFunction<translator::ObjectBuilder::ctor_prototype> object_ctor;
};

TEST_F(Parser, on_data)
{
  EXPECT_CALL(object_ctor,
              Call(testing::Truly([](translator::Environment& env) {
                const auto& obj = env.object_pool.get("/", env);
                return obj.get_value("method", "") == "GET" &&
                       obj.get_value("url", "") == "/";
              })))
    .Times(2)
    .WillRepeatedly(testing::Invoke(
      [] { return std::make_unique<translator::JsonObject>(); }));

  sch->spawn([this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    auto conn = std::make_shared<MockConnection>(sch, co);

    EXPECT_CALL(*conn,
                send(testing::StrEq(
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: application/json; charset=utf-8\r\n\r\n")))
      .Times(2)
      .WillRepeatedly(testing::Return());

    auto parser = parser_factory->create(
      sch, co, std::static_pointer_cast<translator::Connection>(conn));

    parser->on_data("GET / HTTP/1.1\r\n\r\n");
    parser->on_data("GET / HTTP/1.1\r\n\r\n");
  });

  th = std::thread([this] { ios.run(); });

  ios.stop();
  th.join();
}

TEST_F(Parser, on_data_not_found)
{

  sch->spawn([this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    auto conn = std::make_shared<MockConnection>(sch, co);
    EXPECT_CALL(*conn, send(testing::StrEq("HTTP/1.1 404 NOT_FOUND\r\n\r\n")))
      .Times(2)
      .WillRepeatedly(testing::Return());

    auto parser = parser_factory->create(
      sch, co, std::static_pointer_cast<translator::Connection>(conn));

    parser->on_data("GET /root HTTP/1.1\r\n\r\n");
    parser->on_data("GET /root HTTP/1.1\r\n\r\n");
  });

  th = std::thread([this] { ios.run(); });

  ios.stop();
  th.join();
}

TEST_F(Parser, on_data_fail)
{
  EXPECT_CALL(object_ctor,
              Call(testing::Truly([](translator::Environment& env) {
                const auto& obj = env.object_pool.get("/", env);
                return obj.get_value("method", "") == "GET" &&
                       obj.get_value("url", "") == "/";
              })))
    .WillOnce(testing::Return(std::make_unique<translator::JsonObject>()));

  sch->spawn([this](translator::ScheduleRef sch, translator::CoroutineRef co) {
    auto conn = std::make_shared<MockConnection>(sch, co);
    EXPECT_CALL(*conn,
                send(testing::StrEq(
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: application/json; charset=utf-8\r\n\r\n")))
      .WillOnce(testing::Return());

    auto parser = parser_factory->create(
      sch, co, std::static_pointer_cast<translator::Connection>(conn));

    parser->on_data("GET HTTP/1.1\r\n\r\n");
    parser->on_data("GET / HTTP/1.1\r\n\r\n");
  });

  th = std::thread([this] { ios.run(); });

  ios.stop();
  th.join();
}
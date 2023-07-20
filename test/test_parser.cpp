
#include "gmock/gmock.h"
#include <boost/asio/io_service.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include "builder.hpp"
#include "context.hpp"
#include "environment.hpp"
#include "impl/http_parser.hpp"
#include "impl/json_message.hpp"
#include "message.hpp"
#include "mock/mock_builder.hpp"
#include "mock/mock_parser.hpp"
#include "parser.hpp"
#include "schedule.hpp"

class Parser : public testing::Test
{
public:
  Parser()
    : sch(std::make_shared<translator::Schedule>(ios_))
    , parser_factory(std::make_shared<translator::HttpParserFactory>())
    , message_builder(std::make_shared<translator::MessageBuilder>())
  {
    translator::Context::get_instance().message_builder = message_builder;
    message_builder->registe("GET /", message_ctor.AsStdFunction());
  }

public:
  virtual void SetUp()
  {
    work_ = std::make_unique<boost::asio::io_service::work>(ios_);
    th_ = std::thread([this] { ios_.run(); });
  }

  virtual void TearDown()
  {
    work_.reset();
    th_.join();
  }

private:
  boost::asio::io_service ios_;
  std::unique_ptr<boost::asio::io_service::work> work_;
  std::thread th_;

protected:
  std::shared_ptr<translator::Schedule> sch;
  std::shared_ptr<translator::ParserFactory> parser_factory;
  std::shared_ptr<translator::MessageBuilder> message_builder;
  testing::MockFunction<translator::MessageBuilder::ctor_prototype>
    message_ctor;
};

TEST_F(Parser, on_data)
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
}

TEST_F(Parser, on_data_fail)
{
  EXPECT_CALL(
    message_ctor,
    Call(testing::_, testing::Truly([](translator::MessagePtr message) {
           const auto& body = message->get_body();
           return body.get_value("method", "") == "GET" &&
                  body.get_value("url", "") == "/";
         })))
    .WillOnce(testing::Return(std::make_shared<translator::JsonMessage>()));

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
}

#include <chrono>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <thread>

#include "plugin.hpp"
#include "tool/fix_server.hpp"
#include "tool/http_client.hpp"

class Application : public testing::Test
{
public:
  Application()
    : fix_server("../../test/cfg/executor.cfg")
    , fix_client("../src/plugin/fix_client/libfix_client.so.0",
                 { "/usr/local/share/quickfix/FIX42.xml",
                   "../../cfg/fix_client/tradeclient.cfg" })
    , http_to_fix("../src/plugin/http_to_fix/libhttp_to_fix.so.0")
    , http_server("../src/plugin/http_server/libhttp_server.so.0",
                  { "server.sock" })
    , http_client("server.sock")
  {
  }

protected:
  FixServer fix_server;
  amphisbaena::Plugin fix_client;
  amphisbaena::Plugin http_to_fix;
  amphisbaena::Plugin http_server;
  HttpClient http_client;
};

TEST_F(Application, send)
{
  std::promise<void> pms;

  rapidjson::Document request_body(rapidjson::Type::kObjectType);
  rapidjson::Value request_body_head(rapidjson::Type::kObjectType);
  rapidjson::Value request_body_body(rapidjson::Type::kObjectType);

  request_body_head.AddMember("MsgType", "D", request_body.GetAllocator())
    .AddMember("BeginString", "FIX.4.2", request_body.GetAllocator())
    .AddMember("SenderCompID", "CLIENT1", request_body.GetAllocator())
    .AddMember("TargetCompID", "EXECUTOR", request_body.GetAllocator());

  request_body_body.AddMember("ClOrdID", "100001", request_body.GetAllocator())
    .AddMember("HandlInst", "1", request_body.GetAllocator())
    .AddMember("OrdType", "1", request_body.GetAllocator())
    .AddMember("Symbol", "AAPL", request_body.GetAllocator())
    .AddMember("Side", "1", request_body.GetAllocator())
    .AddMember("TransactTime",
               "20230718-04:57:20.922010000",
               request_body.GetAllocator());

  request_body.AddMember(
    "head", request_body_head, request_body.GetAllocator());
  request_body.AddMember(
    "body", request_body_body, request_body.GetAllocator());

  rapidjson::StringBuffer sb;
  rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
  request_body.Accept(writer);

  EXPECT_CALL(fix_server, on_recv("FIX.4.2", "EXECUTOR", "CLIENT1", testing::_))
    .WillOnce(testing::Invoke([this](std::string_view begin_string,
                                     std::string_view sender_comp_id,
                                     std::string_view target_comp_id,
                                     std::string_view) {
      FIX::Message response;
      auto& head = response.getHeader();
      head.setField(FIX::FIELD::MsgType, FIX::MsgType_ExecutionReport);
      head.setField(FIX::FIELD::BeginString, std::string(begin_string));
      head.setField(FIX::FIELD::SenderCompID, std::string(sender_comp_id));
      head.setField(FIX::FIELD::TargetCompID, std::string(target_comp_id));

      auto& body = response;
      body.setField(FIX::FIELD::OrderID, "100001");
      body.setField(FIX::FIELD::ClOrdID, "100001");
      body.setField(FIX::FIELD::ExecID, "100001");
      body.setField(FIX::FIELD::ExecTransType, "0");
      body.setField(FIX::FIELD::ExecType, "0");
      body.setField(FIX::FIELD::OrdStatus, "0");
      body.setField(FIX::FIELD::Symbol, "AAPL");
      body.setField(FIX::FIELD::Side, "1");
      body.setField(FIX::FIELD::LeavesQty, "100.78");
      body.setField(FIX::FIELD::CumQty, "88.88");
      body.setField(FIX::FIELD::AvgPx, "10.01");

      fix_server.send(
        begin_string, sender_comp_id, target_comp_id, response.toString());
    }));

  EXPECT_CALL(http_client, on_recv(200, testing::_))
    .WillOnce(testing::Invoke([&pms](uint16_t, std::string_view) {
      pms.set_value();
    }));

  http_client.send("1.1", "GET", "/", { sb.GetString(), sb.GetLength() });

  pms.get_future().wait_for(std::chrono::milliseconds(5));
}
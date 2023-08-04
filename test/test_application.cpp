
#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
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
  rapidjson::Document request_body(rapidjson::Type::kObjectType);
  request_body.AddMember("BeginString", "FIX.4.2", request_body.GetAllocator())
    .AddMember("SenderCompID", "CLIENT1", request_body.GetAllocator())
    .AddMember("TargetCompID", "EXECUTOR", request_body.GetAllocator());

  rapidjson::StringBuffer sb;
  rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
  request_body.Accept(writer);

  http_client.send("1.1", "GET", "/", { sb.GetString(), sb.GetLength() });

  std::this_thread::sleep_for(std::chrono::seconds(1));
}
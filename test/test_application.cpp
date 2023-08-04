
#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
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
                 { "../../cfg/fix_client/tradeclient.cfg" })
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

TEST_F(Application, send) {
}

#include <chrono>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <quickfix/DataDictionary.h>
#include <quickfix/FixValues.h>
#include <quickfix/SessionID.h>
#include <quickfix/SessionSettings.h>
#include <sstream>
#include <thread>

#include "impl/fix_client.hpp"
#include "impl/fix_message.hpp"
#include "tool/fix_server.hpp"

class FixClient : public testing::Test
{
public:
  void SetUp() {}

  void TearDown() {}
};

TEST_F(FixClient, create)
{
  std::stringstream server_settings(R"(
[DEFAULT]
ConnectionType=acceptor
SocketAcceptPort=10000
SocketReuseAddress=Y
StartTime=00:00:00
EndTime=00:00:00
#FileLogPath=log
UseDataDictionary=Y
#ServerCertificateFile=./cfg/certs/127_0_0_1_server.crt
#ServerCertificateKeyFile=./cfg/certs/127_0_0_1_server.key
SSLProtocol = all
TimestampPrecision=6
PreserveMessageFieldsOrder=N

[SESSION]
BeginString=FIX.4.2
SenderCompID=EXECUTOR
TargetCompID=CLIENT1
#FileStorePath=store
DataDictionary=/usr/local/share/quickfix/FIX42.xml

[SESSION]
BeginString=FIX.4.2
SenderCompID=EXECUTOR
TargetCompID=CLIENT2
#FileStorePath=store
DataDictionary=/usr/local/share/quickfix/FIX42.xml

[SESSION]
BeginString=FIX.4.3
SenderCompID=EXECUTOR
TargetCompID=CLIENT1
#FileStorePath=store
DataDictionary=/usr/local/share/quickfix/FIX43.xml

[SESSION]
BeginString=FIX.4.3
SenderCompID=EXECUTOR
TargetCompID=CLIENT2
#FileStorePath=store
DataDictionary=/usr/local/share/quickfix/FIX43.xml
)");

  std::stringstream client_settings(R"(
[DEFAULT]
ConnectionType=initiator
ReconnectInterval=2
#FileStorePath=store
#FileLogPath=log
StartTime=00:00:00
EndTime=00:00:00
UseDataDictionary=Y
DataDictionary=/usr/local/share/quickfix/FIX42.xml
#HttpAcceptPort=9911
#ClientCertificateFile =
#ClientCertificateKeyFile =
SSLProtocol = +SSLv3 +TLSv1 -SSLv2
PreserveMessageFieldsOrder=6
PreserveMessageFieldsOrder=N

#standard config elements

[SESSION]
#inherit ConnectionType, ReconnectInterval and SenderCompID from default
BeginString=FIX.4.2
SenderCompID=CLIENT1
TargetCompID=EXECUTOR
SocketConnectHost=127.0.0.1
SocketConnectPort=10000
HeartBtInt=30 
)");

  FixServer server(server_settings);
  translator::FixClient client(client_settings);

  std::promise<void> pms;

  EXPECT_CALL(
    server,
    onLogon(testing::Eq(FIX::SessionID("FIX.4.2", "EXECUTOR", "CLIENT1"))))
    .WillOnce(
      testing::Invoke([&pms](const FIX::SessionID&) { pms.set_value(); }));
  EXPECT_CALL(
    server,
    onAdmin(testing::_,
            testing::Eq(FIX::SessionID("FIX.4.2", "EXECUTOR", "CLIENT1"))))
    .WillRepeatedly(testing::Return());
  EXPECT_CALL(
    server,
    onApp(testing::_,
          testing::Eq(FIX::SessionID("FIX.4.2", "EXECUTOR", "CLIENT1"))))
    .WillOnce(testing::Return());
  EXPECT_CALL(server, onLogout(testing::_)).WillOnce(testing::Return());
  server.start();
  client.start();

  pms.get_future().wait_for(std::chrono::milliseconds(10));

  auto session = client.get_session("FIX.4.2", "CLIENT1", "EXECUTOR");

  auto msg = session.new_message();
  auto& head = msg->get_head();
  head.set_value("MsgType", FIX::MsgType_NewOrderSingle);
  head.set_value("BeginString", "FIX.4.2");
  head.set_value("SenderCompID", "CLIENT1");
  head.set_value("TargetCompID", "EXECUTOR");

  auto& body = msg->get_body();
  body.set_value("ClOrdID", "100001");
  body.set_value("HandlInst", "1");
  body.set_value("OrdType", "1");
  body.set_value("Symbol", "AAPL");
  body.set_value("Side", "1");
  body.set_value("TransactTime", "20230718-04:57:20.922010000");

  session.send(*msg);
}
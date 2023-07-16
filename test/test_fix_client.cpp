
#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sstream>
#include <thread>

#include "detail/fix_client.hpp"
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
SocketAcceptPort=10001
SocketReuseAddress=Y
StartTime=00:00:00
EndTime=00:00:00
#FileLogPath=log
UseDataDictionary=Y
ServerCertificateFile=./cfg/certs/127_0_0_1_server.crt
ServerCertificateKeyFile=./cfg/certs/127_0_0_1_server.key
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
FileStorePath=store
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
HttpAcceptPort=9911
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
}
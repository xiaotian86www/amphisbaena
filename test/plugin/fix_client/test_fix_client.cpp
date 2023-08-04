
#include <chrono>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sstream>
#include <thread>

#include "message.hpp"
#include "mock_client.hpp"
#include "plugin/fix_client/fix_client.hpp"
#include "plugin/fix_client/fix_message.hpp"
#include "tool/fix_server.hpp"

class FixClient : public testing::Test
{
public:
  FixClient()
    : client_settings(R"(
[DEFAULT]
ConnectionType=initiator
ReconnectInterval=2
#FileStorePath=store
FileLogPath=log
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
)")
    , server("../../test/cfg/executor.cfg")
  {
    amphisbaena::FixMessage::init("/usr/local/share/quickfix/FIX42.xml");
  }

protected:
  std::stringstream client_settings;
  MockClient::MockMessageHandler message_handler;
  FixServer server;
};

TEST_F(FixClient, send)
{
  std::promise<void> pms;

  EXPECT_CALL(server, on_recv("FIX.4.2", "EXECUTOR", "CLIENT1", testing::_))
    .WillOnce(testing::Invoke([&pms](std::string_view,
                                     std::string_view,
                                     std::string_view,
                                     std::string_view) { pms.set_value(); }));

  auto request = std::make_shared<amphisbaena::FixMessage>();
  auto head = request->get_head();
  head->set_value("MsgType", FIX::MsgType_NewOrderSingle);
  head->set_value("BeginString", "FIX.4.2");
  head->set_value("SenderCompID", "CLIENT1");
  head->set_value("TargetCompID", "EXECUTOR");

  auto body = request->get_body();
  body->set_value("ClOrdID", "100001");
  body->set_value("HandlInst", "1");
  body->set_value("OrdType", "1");
  body->set_value("Symbol", "AAPL");
  body->set_value("Side", "1");
  body->set_value("TransactTime", "20230718-04:57:20.922010000");

  amphisbaena::FixClient client(FIX::SessionSettings(client_settings),
                                &message_handler);

  auto session = client.create(request);

  session->send(request);

  pms.get_future().wait_for(std::chrono::milliseconds(10));
}

TEST_F(FixClient, recv)
{
  std::promise<void> pms;

  EXPECT_CALL(message_handler,
              on_recv(testing::_, testing::_, testing::_, testing::_))
    .WillOnce(
      testing::Invoke([&pms](amphisbaena::ScheduleRef,
                             amphisbaena::CoroutineRef,
                             amphisbaena::SessionPtr,
                             amphisbaena::MessagePtr) { pms.set_value(); }));

  FIX::Message response;
  auto& head = response.getHeader();
  head.setField(FIX::FIELD::MsgType, FIX::MsgType_ExecutionReport);
  head.setField(FIX::FIELD::BeginString, FIX::BeginString_FIX42);
  head.setField(FIX::FIELD::SenderCompID, "EXECUTOR");
  head.setField(FIX::FIELD::TargetCompID, "CLIENT1");

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

  amphisbaena::FixClient client(FIX::SessionSettings(client_settings),
                                &message_handler);

  server.send("FIX.4.2", "EXECUTOR", "CLIENT1", response.toString());

  pms.get_future().wait_for(std::chrono::milliseconds(10));
}
#pragma once

#include <gmock/gmock.h>
#include <istream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"
#include <quickfix/Acceptor.h>
#include <quickfix/Application.h>
#include <quickfix/Log.h>
#include <quickfix/SessionID.h>
#include <quickfix/SessionSettings.h>
#pragma GCC diagnostic pop

class FixServer : public FIX::Application
{
public:
  FixServer(const FIX::SessionSettings& settings);

  ~FixServer() override;

public:
  void start();

  void stop();

  void send(FIX::Message& message);

  MOCK_METHOD(void, onLogon, (const FIX::SessionID&), (override));

  MOCK_METHOD(void, onLogout, (const FIX::SessionID&), (override));

  MOCK_METHOD(void, onAdmin, (const FIX::Message&, const FIX::SessionID&), ());

  MOCK_METHOD(void, onApp, (const FIX::Message&, const FIX::SessionID&), ());

public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"
  /// Notification of a session begin created
  void onCreate(const FIX::SessionID&) override;
  /// Notification of admin message being sent to target
  void toAdmin(FIX::Message&, const FIX::SessionID&) override;
  /// Notification of app message being sent to target
  void toApp(FIX::Message&,
             const FIX::SessionID&) throw(FIX::DoNotSend) override;
  /// Notification of admin message being received from target
  void fromAdmin(const FIX::Message&,
                 const FIX::SessionID&) throw(FIX::FieldNotFound,
                                              FIX::IncorrectDataFormat,
                                              FIX::IncorrectTagValue,
                                              FIX::RejectLogon) override;
  /// Notification of app message being received from target
  void fromApp(const FIX::Message&, const FIX::SessionID&) throw(
    FIX::FieldNotFound,
    FIX::IncorrectDataFormat,
    FIX::IncorrectTagValue,
    FIX::UnsupportedMessageType) override;
#pragma GCC diagnostic pop
private:
  std::unique_ptr<FIX::MessageStoreFactory> store_factory_;
  std::unique_ptr<FIX::LogFactory> log_factory_;
  std::unique_ptr<FIX::Acceptor> acceptor_;
};

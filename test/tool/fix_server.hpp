#pragma once

#include <istream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"
#include <quickfix/Application.h>
#include <quickfix/Acceptor.h>
#include <quickfix/Log.h>
#include <quickfix/SessionSettings.h>
#pragma GCC diagnostic pop

class FixServer
  : public FIX::Application
{
public:
  FixServer(std::istream& is);

  ~FixServer() override;

public:
  void start();

  void stop();

public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"
  /// Notification of a session begin created
  void onCreate(const FIX::SessionID&) override;
  /// Notification of a session successfully logging on
  void onLogon(const FIX::SessionID&) override;
  /// Notification of a session logging off or disconnecting
  void onLogout(const FIX::SessionID&) override;
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
  FIX::SessionSettings settings_;
  std::unique_ptr<FIX::MessageStoreFactory> store_factory_;
  std::unique_ptr<FIX::LogFactory> log_factory_;
  std::unique_ptr<FIX::Acceptor> acceptor_;
};
#pragma once

#include <istream>
#include <map>
#include <memory>
#include <string_view>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"
#include <quickfix/Application.h>
#include <quickfix/DataDictionary.h>
#include <quickfix/Initiator.h>
#include <quickfix/Log.h>
#include <quickfix/Message.h>
#include <quickfix/Session.h>
#include <quickfix/SessionID.h>
#include <quickfix/SessionSettings.h>
#pragma GCC diagnostic pop

#include "fix_message.hpp"
#include "message.hpp"
#include "client.hpp"

namespace translator {

class FixSession : public Session
{
public:
  FixSession(FIX::Session* session)
    : session_(session)
  {
  }

public:
  void send(MessagePtr data) override;

private:
  FIX::Session* session_;
};

class FixClient
  : public Client
  , public FIX::Application
{
public:
  FixClient(std::istream& is);

  ~FixClient();

public:
  void start() override;

  void stop() override;

  SessionPtr create(MessagePtr message) override;

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
  void init_sessions();

private:
  FIX::SessionSettings settings_;
  std::unique_ptr<FIX::MessageStoreFactory> store_factory_;
  std::unique_ptr<FIX::LogFactory> log_factory_;
  std::unique_ptr<FIX::Initiator> initiator_;
  std::map<FIX::SessionID, std::shared_ptr<FixSession>> sessions_;
};
}
#pragma once

#include "schedule.hpp"
#include <filesystem>
#include <istream>
#include <map>
#include <memory>
#include <string_view>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-copy"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <quickfix/Application.h>
#include <quickfix/DataDictionary.h>
#include <quickfix/Initiator.h>
#include <quickfix/Log.h>
#include <quickfix/Message.h>
#include <quickfix/Session.h>
#include <quickfix/SessionID.h>
#include <quickfix/SessionSettings.h>
#pragma GCC diagnostic pop

#include "client.hpp"
#include "fix_message.hpp"
#include "message.hpp"

namespace amphisbaena {

class FixSession : public Session
{
public:
  FixSession(FIX::Session* session, MessageHandler& message_handler)
    : Session(ScheduleRef(), CoroutineRef(), message_handler)
    , session_(session)
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
  FixClient(const FIX::SessionSettings& settings, Session::MessageHandler& handler);

  ~FixClient() override;

public:
  SessionPtr create(MessagePtr message) override;

public:
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
             const FIX::SessionID&) EXCEPT(FIX::DoNotSend) override;
  /// Notification of admin message being received from target
  void fromAdmin(const FIX::Message&,
                 const FIX::SessionID&) EXCEPT(FIX::FieldNotFound,
                                              FIX::IncorrectDataFormat,
                                              FIX::IncorrectTagValue,
                                              FIX::RejectLogon) override;
  /// Notification of app message being received from target
  void fromApp(const FIX::Message&, const FIX::SessionID&) EXCEPT(
    FIX::FieldNotFound,
    FIX::IncorrectDataFormat,
    FIX::IncorrectTagValue,
    FIX::UnsupportedMessageType) override;

private:
  void init_sessions();

private:
  FIX::SessionSettings settings_;
  std::unique_ptr<FIX::MessageStoreFactory> store_factory_;
  std::unique_ptr<FIX::LogFactory> log_factory_;
  std::unique_ptr<FIX::Initiator> initiator_;
  std::map<FIX::SessionID, std::shared_ptr<FixSession>> sessions_;
};

class FixClientFactory : public ClientFactory
{
public:
  FixClientFactory(std::istream& is);

  FixClientFactory(const std::filesystem::path& pt);

public:
  std::unique_ptr<Client> create(Session::MessageHandler& handler) override;

private:
  FIX::SessionSettings settings_;
};
}
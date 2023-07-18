#pragma once

#include <istream>
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
#include <quickfix/SessionSettings.h>
#pragma GCC diagnostic pop

#include "fix_message.hpp"
#include "message.hpp"
#include "service.hpp"

namespace translator {

class FixSession
{
public:
  FixSession(FIX::Session& session);

public:
  std::unique_ptr<FixMessage> new_message();

  void send(FixMessage& message);

private:
  FIX::Session& session_;
};

class FixClient
  : public Service
  , public FIX::Application
{
public:
  FixClient(std::istream& is);

  ~FixClient();

public:
  void start() override;

  void stop() override;

public:
  FixSession get_session(std::string_view begin_string,
                         std::string_view sender_comp_id,
                         std::string_view target_comp_id);

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
  std::unique_ptr<FIX::Initiator> initiator_;
};
}
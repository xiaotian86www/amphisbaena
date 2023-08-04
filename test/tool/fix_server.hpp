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
  void send(std::string_view begin_string,
            std::string_view sender_comp_id,
            std::string_view target_comp_id,
            std::string_view body);

  MOCK_METHOD(void,
              on_recv,
              (std::string_view begin_string,
               std::string_view sender_comp_id,
               std::string_view target_comp_id,
               std::string_view body),
              ());

public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"
  void onLogon(const FIX::SessionID&) override;
  void onLogout(const FIX::SessionID&) override;
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

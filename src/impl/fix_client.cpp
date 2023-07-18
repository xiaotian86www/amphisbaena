#include <istream>
#include <memory>
#include <stdexcept>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"
#include <quickfix/DataDictionary.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/FixFieldNumbers.h>
#include <quickfix/FixFields.h>
#include <quickfix/Log.h>
#include <quickfix/Message.h>
#include <quickfix/MessageStore.h>
#include <quickfix/Session.h>
#include <quickfix/SessionID.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>
#pragma GCC diagnostic pop

#include "fix_client.hpp"

namespace translator {

FixSession::FixSession(FIX::Session& session)
  : session_(session)
{
}

std::unique_ptr<FixMessage>
FixSession::new_message()
{
  return std::make_unique<FixMessage>(
    session_.getDataDictionaryProvider().getSessionDataDictionary(
      session_.getSessionID().getBeginString()));
}

void
FixSession::send(FixMessage& message)
{
  session_.send(message.message());
}

FixClient::FixClient(std::istream& is)
  : settings_(is)
{
  if (settings_.get().has(FIX::FILE_STORE_PATH))
    store_factory_ = std::make_unique<FIX::FileStoreFactory>(settings_);
  else
    store_factory_ = std::make_unique<FIX::MemoryStoreFactory>();

  if (settings_.get().has(FIX::FILE_LOG_PATH))
    log_factory_ = std::make_unique<FIX::FileLogFactory>(settings_);
  else
    log_factory_ = std::make_unique<FIX::ScreenLogFactory>(settings_);

  initiator_ = std::make_unique<FIX::SocketInitiator>(
    *this, *store_factory_, settings_, *log_factory_);
}

FixClient::~FixClient()
{
  stop();
}

void
FixClient::start()
{
  initiator_->start();
}

void
FixClient::stop()
{
  initiator_->stop();
}

FixSession
FixClient::get_session(std::string_view begin_string,
                       std::string_view sender_comp_id,
                       std::string_view target_comp_id)
{
  auto session =
    initiator_->getSession(FIX::SessionID(std::string(begin_string),
                                          std::string(sender_comp_id),
                                          std::string(target_comp_id)));
  if (!session) {
    throw std::invalid_argument(
      "invalid begin_string sender_comp_id and target_comp_id");
  }

  return FixSession(*session);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"
void
FixClient::onCreate(const FIX::SessionID&)
{
}

void
FixClient::onLogon(const FIX::SessionID&)
{
}

void
FixClient::onLogout(const FIX::SessionID&)
{
}

void
FixClient::toAdmin(FIX::Message&, const FIX::SessionID&)
{
}

void
FixClient::toApp(FIX::Message&, const FIX::SessionID&) throw(FIX::DoNotSend)
{
}

void
FixClient::fromAdmin(const FIX::Message&,
                     const FIX::SessionID&) throw(FIX::FieldNotFound,
                                                  FIX::IncorrectDataFormat,
                                                  FIX::IncorrectTagValue,
                                                  FIX::RejectLogon)
{
}

void
FixClient::fromApp(const FIX::Message&,
                   const FIX::SessionID&) throw(FIX::FieldNotFound,
                                                FIX::IncorrectDataFormat,
                                                FIX::IncorrectTagValue,
                                                FIX::UnsupportedMessageType)
{
}
#pragma GCC diagnostic pop
}
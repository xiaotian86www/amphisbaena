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

#include "detail/fix_client.hpp"

namespace translator {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"

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

void
FixClient::send(FixObject& obj)
{
  FIX::Session::sendToTarget(obj.message());
  // FIX::Message message;
  // auto begin_string = std::string(obj.get_string(FIX::BEGINSTRING));
  // auto sender_comp_id = std::string(obj.get_string(FIX::SENDERCOMPID));
  // auto target_comp_id = std::string(obj.get_string(FIX::TARGETCOMPID));

  // auto session_id =
  //   FIX::SessionID(begin_string, sender_comp_id, target_comp_id);
  // auto session = FIX::Session::lookupSession(session_id);
  // if (!session)
  //   throw std::invalid_argument("session not found, please check begin_string, "
  //                               "sender_comp_id and target_comp_id");

  // const auto& ddp = session->getDataDictionaryProvider();
  // const auto& dd = ddp.getSessionDataDictionary(begin_string);
}

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
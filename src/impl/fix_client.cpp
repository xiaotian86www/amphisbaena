#include "fix_message.hpp"
#include <cassert>
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

void
FixClient::send(MessagePtr message)
{
  FIX::Session::sendToTarget(
    std::static_pointer_cast<FixMessage>(message)->message());
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
FixClient::fromApp(const FIX::Message& message,
                   const FIX::SessionID&) throw(FIX::FieldNotFound,
                                                FIX::IncorrectDataFormat,
                                                FIX::IncorrectTagValue,
                                                FIX::UnsupportedMessageType)
{
  auto response = std::make_shared<FixMessage>(message);
  assert(handler);
  handler->on_message(response);
}
#pragma GCC diagnostic pop
}
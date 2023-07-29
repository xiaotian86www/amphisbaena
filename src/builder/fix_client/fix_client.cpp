
#include <cassert>
#include <istream>
#include <memory>
#include <mutex>
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

#include "../../impl/fix_message.hpp"
#include "client.hpp"
#include "fix_client.hpp"
#include "schedule.hpp"

namespace translator {

void
FixSession::send(MessagePtr data)
{
  assert(session_);
  session_->send(std::static_pointer_cast<FixMessage>(data)->fix_message);
}

FixClient::FixClient(std::istream& is)
  : FixClient(FIX::SessionSettings(is))
{
}

FixClient::FixClient(const std::filesystem::path& pt)
  : FixClient(FIX::SessionSettings(pt.string()))
{
}

FixClient::FixClient(FIX::SessionSettings settings)
  : settings_(std::move(settings))
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
  init_sessions();
  initiator_->start();
}

void
FixClient::stop()
{
  initiator_->stop();
}

SessionPtr
FixClient::create(MessagePtr message)
{
  auto head = message->get_head();
  FIX::SessionID session_id(std::string(head->get_string("BeginString")),
                            std::string(head->get_string("SenderCompID")),
                            std::string(head->get_string("TargetCompID")));

  if (auto iter = sessions_.find(session_id); iter != sessions_.end()) {
    return iter->second;
  } else {
    return SessionPtr();
  }
}

// void
// FixClient::send(MessagePtr message)
// {
//   FIX::Session::sendToTarget(
//     std::static_pointer_cast<FixMessage>(message)->message());
// }

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
FixClient::fromApp(
  const FIX::Message& message,
  const FIX::SessionID& session_id) throw(FIX::FieldNotFound,
                                          FIX::IncorrectDataFormat,
                                          FIX::IncorrectTagValue,
                                          FIX::UnsupportedMessageType)
{
  auto iter = sessions_.find(session_id);
  if (iter == sessions_.end())
    return;

  SessionPtr session = iter->second;

  auto response = std::make_shared<FixMessage>(message);
  assert(message_handler);
  message_handler->on_recv(
    translator::ScheduleRef(), translator::CoroutineRef(), session, response);
}
#pragma GCC diagnostic pop

void
FixClient::init_sessions()
{
  auto session_ids = FIX::Session::getSessions();
  for (const auto& session_id : session_ids) {
    auto session = FIX::Session::lookupSession(session_id);
    auto fix_session = std::make_shared<FixSession>(session);
    sessions_.insert_or_assign(session_id, fix_session);
  }
}
}
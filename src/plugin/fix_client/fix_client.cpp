
#include <cassert>
#include <istream>
#include <memory>
#include <mutex>
#include <stdexcept>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wdeprecated-copy-with-user-provided-copy"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
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

#include "client.hpp"
#include "fix_client.hpp"
#include "fix_message.hpp"
#include "log.hpp"
#include "schedule.hpp"

namespace amphisbaena {

void
FixSession::send(MessagePtr data)
{
  assert(session_);
  session_->send(std::static_pointer_cast<FixMessage>(data)->fix_message);
}

FixClient::FixClient(const FIX::SessionSettings& settings,
                     MessageHandler* handler)
  : Client(handler)
  , settings_(std::move(settings))
{
  LOG_INFO("FixClient create");
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

  init_sessions();
  initiator_->start();
}

FixClient::~FixClient()
{
  LOG_INFO("FixClient destroy");
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

void
FixClient::onCreate(const FIX::SessionID&)
{
}

void
FixClient::onLogon(const FIX::SessionID& session_id)
{
  LOG_INFO("Logon begin_string: {}, sender_comp_id: {}, target_comp_id: {}",
           session_id.getBeginString().getString(),
           session_id.getSenderCompID().getString(),
           session_id.getTargetCompID().getString());
}

void
FixClient::onLogout(const FIX::SessionID& session_id)
{
  LOG_INFO("Logout begin_string: {}, sender_comp_id: {}, target_comp_id: {}",
           session_id.getBeginString().getString(),
           session_id.getSenderCompID().getString(),
           session_id.getTargetCompID().getString());
}

void
FixClient::toAdmin(FIX::Message&, const FIX::SessionID&)
{
}

void
FixClient::toApp(FIX::Message&, const FIX::SessionID&) EXCEPT(FIX::DoNotSend)
{
}

void
FixClient::fromAdmin(const FIX::Message&,
                     const FIX::SessionID&) EXCEPT(FIX::FieldNotFound,
                                                  FIX::IncorrectDataFormat,
                                                  FIX::IncorrectTagValue,
                                                  FIX::RejectLogon)
{
}

void
FixClient::fromApp(
  const FIX::Message& message,
  const FIX::SessionID& session_id) EXCEPT(FIX::FieldNotFound,
                                          FIX::IncorrectDataFormat,
                                          FIX::IncorrectTagValue,
                                          FIX::UnsupportedMessageType)
{
  auto iter = sessions_.find(session_id);
  if (iter == sessions_.end())
    return;

  SessionPtr session = iter->second;

  auto response = std::make_shared<FixMessage>(message);
  if (message_handler_)
    message_handler_->on_recv(amphisbaena::ScheduleRef(),
                              amphisbaena::CoroutineRef(),
                              session,
                              response);
}

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

FixClientFactory::FixClientFactory(std::istream& is)
  : settings_(is)
{
}

FixClientFactory::FixClientFactory(const std::filesystem::path& pt)
  : settings_(pt)
{
}

std::unique_ptr<Client>
FixClientFactory::create(Client::MessageHandler* handler)
{
  return std::make_unique<FixClient>(settings_, handler);
}

}
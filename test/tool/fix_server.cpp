#include <istream>
#include <memory>
#include <quickfix/FixFieldNumbers.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/Log.h>
#include <quickfix/MessageStore.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketAcceptor.h>
#pragma GCC diagnostic pop

#include "fix_server.hpp"

FixServer::FixServer(const FIX::SessionSettings& settings)
{
  if (settings.get().has(FIX::FILE_STORE_PATH))
    store_factory_ = std::make_unique<FIX::FileStoreFactory>(settings);
  else
    store_factory_ = std::make_unique<FIX::MemoryStoreFactory>();

  if (settings.get().has(FIX::FILE_LOG_PATH))
    log_factory_ = std::make_unique<FIX::FileLogFactory>(settings);
  else
    log_factory_ = std::make_unique<FIX::ScreenLogFactory>(settings);

  acceptor_ = std::make_unique<FIX::SocketAcceptor>(
    *this, *store_factory_, settings, *log_factory_);
}

FixServer::~FixServer()
{
  stop();
}

void
FixServer::start()
{
  acceptor_->start();
}

void
FixServer::stop()
{
  acceptor_->stop();
}

void
FixServer::send(std::string_view begin_string,
                std::string_view sender_comp_id,
                std::string_view target_comp_id,
                std::string_view body)
{
  FIX::SessionID session_id = FIX::SessionID(std::string(begin_string),
                                             std::string(sender_comp_id),
                                             std::string(target_comp_id));
  FIX::Message message = FIX::Message(std::string(body));
  FIX::Session::sendToTarget(message, session_id);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"
void
FixServer::onCreate(const FIX::SessionID&)
{
}

void
FixServer::toAdmin(FIX::Message&, const FIX::SessionID&)
{
}

void
FixServer::toApp(FIX::Message&, const FIX::SessionID&) throw(FIX::DoNotSend)
{
}

void
FixServer::fromAdmin(
  const FIX::Message& message,
  const FIX::SessionID& session_id) throw(FIX::FieldNotFound,
                                          FIX::IncorrectDataFormat,
                                          FIX::IncorrectTagValue,
                                          FIX::RejectLogon)
{
}

void
FixServer::fromApp(
  const FIX::Message& message,
  const FIX::SessionID& session_id) throw(FIX::FieldNotFound,
                                          FIX::IncorrectDataFormat,
                                          FIX::IncorrectTagValue,
                                          FIX::UnsupportedMessageType)
{
  on_recv(session_id.getBeginString().getString(),
          session_id.getSenderCompID().getString(),
          session_id.getTargetCompID().getString(),
          message.toString());
}
#pragma GCC diagnostic pop
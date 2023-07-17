#include <istream>
#include <memory>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/Log.h>
#include <quickfix/MessageStore.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketAcceptor.h>
#pragma GCC diagnostic pop

#include "fix_server.hpp"

FixServer::FixServer(std::istream& is)
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

  acceptor_ = std::make_unique<FIX::SocketAcceptor>(
    *this, *store_factory_, settings_, *log_factory_);
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"

void
FixServer::onCreate(const FIX::SessionID&)
{
}

void
FixServer::onLogon(const FIX::SessionID&)
{
}

void
FixServer::onLogout(const FIX::SessionID&)
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
FixServer::fromAdmin(const FIX::Message&,
                     const FIX::SessionID&) throw(FIX::FieldNotFound,
                                                  FIX::IncorrectDataFormat,
                                                  FIX::IncorrectTagValue,
                                                  FIX::RejectLogon)
{
}

void
FixServer::fromApp(const FIX::Message&,
                   const FIX::SessionID&) throw(FIX::FieldNotFound,
                                                FIX::IncorrectDataFormat,
                                                FIX::IncorrectTagValue,
                                                FIX::UnsupportedMessageType)
{
}
#pragma GCC diagnostic pop
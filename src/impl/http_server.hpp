#pragma once

#include <llhttp.h>
#include <map>
#include <memory>

#include "builder.hpp"
#include "environment.hpp"
#include "message.hpp"
#include "server.hpp"
#include "session.hpp"

namespace translator {
class HttpServer;

class HttpSession
  : public std::enable_shared_from_this<HttpSession>
  , public Session
  , public llhttp_t
{
public:
  HttpSession(HttpServer* server,
              ScheduleRef sch,
              CoroutineRef co,
              ConnectionRef conn);

public:
  void send(MessagePtr message) override;

  void on_recv(std::string_view data);

  void do_recv();

  void set_head(std::string_view name, std::string_view value);

  void set_body(std::string_view value);

private:
  MessagePtr handle_error(std::string_view version);

private:
  HttpServer* server_;
  Environment env_;
  ScheduleRef sch_;
  CoroutineRef co_;
  ConnectionRef conn_;
  MessagePtr request_;
};

typedef std::shared_ptr<HttpSession> HttpSessionPtr;

class HttpServer : public Server::MessageHandler
{
public:
  HttpServer(std::function<Server::ctor_prototype>
               server_factory);

  ~HttpServer() override;

public:
  void on_recv(ScheduleRef sch,
               CoroutineRef co,
               ConnectionPtr conn,
               std::string_view data) override;

private:
  llhttp_settings_t settings_;
  std::unique_ptr<Server> server_;
  std::map<ConnectionPtr, HttpSessionPtr> sessions_;
};
}
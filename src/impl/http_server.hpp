#pragma once

#include <llhttp.h>
#include <map>
#include <memory>

#include "builder.hpp"
#include "server.hpp"
#include "service.hpp"

namespace translator {
class HttpServer;

class HttpSession
  : public std::enable_shared_from_this<HttpSession>
  , public Session
  , public llhttp_t
{
public:
  HttpSession(HttpServer* server, ConnectionRef conn);

public:
  void send(MessagePtr message) override;

  void on_recv(ScheduleRef sch, CoroutineRef co, std::string_view data);

  void do_recv();

  void set_field(std::string_view name, std::string_view value);

private:
  void handle_error(llhttp_status_t status);

private:
  HttpServer* server_;
  ScheduleRef sch_;
  CoroutineRef co_;
  ConnectionRef conn_;
  MessagePtr message_;
};

typedef std::shared_ptr<HttpSession> HttpSessionPtr;

class HttpServer
  : public Server::MessageHandler
{
public:
  HttpServer(std::unique_ptr<Server> server);

  ~HttpServer() override;

public:
  void start();

  void stop();

  void on_recv(ScheduleRef sch,
               CoroutineRef co,
               ConnectionPtr conn,
               std::string_view data) override;

public:
  MessageBuilderPtr message_builder;

private:
  std::unique_ptr<Server> server_;
  std::map<ConnectionPtr, HttpSessionPtr> sessions_;
};
}
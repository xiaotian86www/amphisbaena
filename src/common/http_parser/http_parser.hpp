#pragma once

#include <llhttp.h>
#include <map>
#include <memory>

#include "builder.hpp"
#include "environment.hpp"
#include "message.hpp"
#include "server.hpp"
#include "session.hpp"

namespace amphisbaena {
class HttpServer;

// struct content_t
// {
//   std::size_t capital;
//   std::size_t length;
//   char buffer[];
// };

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

  ~HttpSession() override;

public:
  void send(MessagePtr message) override;

  void on_recv(std::string_view data);

private:
  static int handle_on_message_begin(llhttp_t* http);

  static int handle_on_method(llhttp_t* http, const char* at, size_t length);

  static int handle_on_url(llhttp_t* http, const char* at, size_t length);

  static int handle_on_version(llhttp_t* http, const char* at, size_t length);

  static int handle_on_headers_complete(llhttp_t* http);

  static int handle_on_body(llhttp_t* http, const char* at, size_t length);

  static int handle_on_message_complete(llhttp_t* http);

private:
  MessagePtr handle_error(std::string_view version);

private:
  static llhttp_settings_t settings_;
  HttpServer* server_;
  Environment env_;
  ScheduleRef sch_;
  CoroutineRef co_;
  ConnectionRef conn_;
  MessagePtr request_;
  std::string content_;
};

typedef std::shared_ptr<HttpSession> HttpSessionPtr;

class HttpServer : public Server::MessageHandler
{
public:
  HttpServer(std::shared_ptr<ServerFactory> server_factory);

  ~HttpServer() override;

public:
  void on_recv(ScheduleRef sch,
               CoroutineRef co,
               ConnectionPtr conn,
               std::string_view data) override;

private:
  std::unique_ptr<Server> server_;
  std::map<ConnectionPtr, HttpSessionPtr> sessions_;
};
}
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

class HttpSession
  : public Session
  , public llhttp_t
{
public:
  HttpSession(ScheduleRef sch,
              CoroutineRef co,
              ConnectionRef conn,
              Session::MessageHandler& message_handle);

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
  Environment env_;
  ConnectionRef conn_;
  MessagePtr request_;
  std::string content_;
};

typedef std::shared_ptr<HttpSession> HttpSessionPtr;

class HttpServer : public Connection::MessageHandler
{
public:
  HttpServer(std::shared_ptr<ServerFactory> server_factory,
             Session::MessageHandler& message_handler);

  ~HttpServer() override;

public:
  void on_recv(ScheduleRef sch,
               CoroutineRef co,
               ConnectionPtr conn,
               std::string_view data) override;

public:
  Server* server() { return server_.get(); }

protected:
  Session::MessageHandler& message_handler_;

private:
  std::unique_ptr<Server> server_;
  std::map<ConnectionPtr, HttpSessionPtr> sessions_;
};
}
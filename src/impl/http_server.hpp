#pragma once

#include <llhttp.h>
#include <memory>

#include "server.hpp"
#include "service.hpp"

namespace translator {
class HttpSession
  : public std::enable_shared_from_this<HttpSession>
  , public Session
  , public llhttp_t
{
public:
  HttpSession();

public:
  void send(MessagePtr data) override;
};

class HttpServer
  : public Server::MessageHandler
  , public Service
{
public:
  HttpServer(std::unique_ptr<Server> server);

  ~HttpServer();

public:
  void start() override;

  void stop() override;

  SessionPtr create(MessagePtr message) override;

  void on_recv(ScheduleRef sch,
               CoroutineRef co,
               ConnectionRef conn,
               std::string_view data) override;

  void handle();

  void set_field(std::string_view name, std::string_view value);

private:
  void handle_error(llhttp_status_t status);

  void handle_success(MessagePtr message);

private:
  std::unique_ptr<Server> server_;
};
}

#include <boost/coroutine/exceptions.hpp>
#include <llhttp.h>
#include <memory>
#include <rapidjson/document.h>

#include "environment.hpp"
#include "http_server.hpp"
#include "json_message.hpp"
#include "message.hpp"
#include "session.hpp"

namespace translator {

static int
handle_on_method(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpSession*>(http);
  parser->set_head("method", std::string_view(at, length));
  return HPE_OK;
}

static int
handle_on_url(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpSession*>(http);
  parser->set_head("url", std::string_view(at, length));
  return HPE_OK;
}

static int
handle_on_version(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpSession*>(http);
  parser->set_head("version", std::string_view(at, length));
  return HPE_OK;
}

static int
handle_on_body(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpSession*>(http);
  parser->set_body(std::string_view(at, length));
  return HPE_OK;
}

static int
handle_on_message_complete(llhttp_t* http)
{
  auto parser = static_cast<HttpSession*>(http);
  parser->do_recv();
  return HPE_OK;
}

HttpSession::HttpSession(HttpServer* server, ConnectionRef conn)
  : server_(server)
  , conn_(conn)
  , request_(std::make_shared<JsonMessage>())
{
}

void
HttpSession::send(MessagePtr message)
{
  std::string response;
  auto head = message->get_head();
  auto body = message->get_body();
  const auto& body_str = static_cast<JsonObject*>(body.get())->to_string();
  // const auto& body = message->get_body();
  response += "HTTP/";
  response += head->get_string("version");
  response += " ";
  response += std::to_string(head->get_int("code"));
  response += " ";
  response +=
    llhttp_status_name(static_cast<llhttp_status_t>(head->get_int("code")));
  response += "\r\n";
  response += "Content-Type: application/json; charset=utf-8\r\n";
  response += "Content-Length: " + std::to_string(body_str.length()) + "\r\n";
  response += "\r\n";
  response += body_str;

  conn_.send(response);
}

void
HttpSession::on_recv(ScheduleRef sch, CoroutineRef co, std::string_view data)
{
  enum llhttp_errno err = llhttp_execute(this, data.data(), data.length());
  if (err != HPE_OK) {
    llhttp_reset(this);
    request_ = std::make_shared<JsonMessage>();
  }
}

void
HttpSession::do_recv()
{
  std::string response_name;
  auto request_head = request_->get_head();
  response_name += request_head->get_string("method");
  response_name += " ";
  response_name += request_head->get_string("url");

  MessagePtr response;
  ObjectPtr response_head;

  Environment env;
  env.sch = sch_;
  env.co = co_;
  env.down = shared_from_this();

  try {
    response = server_->message_builder->create(env, response_name, request_);

    response_head = response->get_head();

    response_head->set_value("code", HTTP_STATUS_OK);

  } catch (const boost::coroutines::detail::forced_unwind&) {
    throw;
  } catch (const NotFoundException& ec) {
    response = std::make_shared<JsonMessage>();
    response_head = response->get_head();

    if (ec.name() == response_name) {
      response_head->set_value("code", HTTP_STATUS_NOT_FOUND);
    } else {
      response_head->set_value("code", HTTP_STATUS_INTERNAL_SERVER_ERROR);
    }
  } catch (...) {
  }

  response_head->set_value("version", request_head->get_string("version"));

  send(response);

  request_ = std::make_shared<JsonMessage>();
}

void
HttpSession::set_head(std::string_view name, std::string_view value)
{
  auto request_head = request_->get_head();
  request_head->set_value(name, value);
}

void
HttpSession::set_body(std::string_view value)
{
  auto request_body = request_->get_body();
  static_cast<JsonObject*>(request_body.get())->from_string(value);
}

HttpServer::HttpServer(std::unique_ptr<Server> server)
  : server_(std::move(server))
{
  server_->message_handler = this;

  llhttp_settings_init(&settings_);
  settings_.on_method = handle_on_method;
  settings_.on_url = handle_on_url;
  settings_.on_version = handle_on_version;
  settings_.on_body = handle_on_body;
  settings_.on_message_complete = handle_on_message_complete;
}

HttpServer::~HttpServer()
{
  stop();
}

void
HttpServer::start()
{
  server_->start();
}

void
HttpServer::stop()
{
  server_->stop();
}

void
HttpServer::on_recv(ScheduleRef sch,
                    CoroutineRef co,
                    ConnectionPtr conn,
                    std::string_view data)
{
  HttpSessionPtr session;
  if (auto iter = sessions_.find(conn); iter == sessions_.end()) {
    session = std::make_shared<HttpSession>(this, conn);
    llhttp_init(session.get(), HTTP_REQUEST, &settings_);
    sessions_.insert_or_assign(conn, session);
  } else {
    session = iter->second;
  }

  session->on_recv(sch, co, data);
}
}
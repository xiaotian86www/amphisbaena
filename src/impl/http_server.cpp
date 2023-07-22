
#include <boost/coroutine/exceptions.hpp>
#include <memory>

#include "environment.hpp"
#include "http_server.hpp"
#include "json_message.hpp"
#include "session.hpp"

namespace translator {

static int
handle_on_method(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpSession*>(http);
  parser->set_field("method", std::string_view(at, length));
  return HPE_OK;
}

static int
handle_on_url(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpSession*>(http);
  parser->set_field("url", std::string_view(at, length));
  return HPE_OK;
}

static int
handle_on_version(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpSession*>(http);
  parser->set_field("version", std::string_view(at, length));
  return HPE_OK;
}

static int
handle_on_body(llhttp_t* http, const char* at, size_t length)
{
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
  , message_(std::make_shared<JsonMessage>())
{
}

void
HttpSession::send(MessagePtr message)
{
  std::string response;
  const auto& body = message->get_body();
  response += "HTTP/";
  response += body.get_value("version", "1.1");
  response += " ";
  response += std::to_string(HTTP_STATUS_OK);
  response += " ";
  response += llhttp_status_name(HTTP_STATUS_OK);
  response += "\r\n";
  response += "Content-Type: application/json; charset=utf-8\r\n";
  // response += "Content-Length: " + std::to_string()
  response += "\r\n";

  conn_.send(response);
}

void
HttpSession::on_recv(ScheduleRef sch, CoroutineRef co, std::string_view data)
{
  enum llhttp_errno err = llhttp_execute(this, data.data(), data.length());
  if (err != HPE_OK) {
    message_->clear();
    llhttp_reset(this);
  }
}

void
HttpSession::do_recv()
{
  std::string response_name;
  const auto& request_body = message_->get_body();
  response_name += request_body.get_string("method");
  response_name += " ";
  response_name += request_body.get_string("url");

  try {
    Environment env;
    env.sch = sch_;
    env.co = co_;
    env.down = shared_from_this();

    auto response =
      server_->message_builder->create(env, response_name, message_);

    message_ = std::make_shared<JsonMessage>();

    send(response);

  } catch (const boost::coroutines::detail::forced_unwind&) {
    throw;
  } catch (const NotFoundException& ec) {
    if (ec.name() == response_name) {
      handle_error(HTTP_STATUS_NOT_FOUND);
    } else {
      handle_error(HTTP_STATUS_INTERNAL_SERVER_ERROR);
    }
  } catch (...) {
  }
}

void
HttpSession::set_field(std::string_view name, std::string_view value)
{
  auto& request_body = message_->get_body();
  request_body.set_value(name, value);
}

void
HttpSession::handle_error(llhttp_status_t status)
{
  std::string response;
  const auto& request_body = message_->get_body();
  response += "HTTP/";
  response += request_body.get_value("version", "1.1");
  response += " ";
  response += std::to_string(status);
  response += " ";
  response += llhttp_status_name(status);
  response += "\r\n\r\n";

  conn_.send(response);
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
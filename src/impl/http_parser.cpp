
#include <boost/coroutine/exceptions.hpp>
#include <iostream>
#include <llhttp.h>
#include <memory>
#include <mutex>
#include <string>

#include "context.hpp"
#include "environment.hpp"
#include "http_parser.hpp"
#include "json_message.hpp"
#include "message.hpp"
#include "parser.hpp"

namespace translator {

static llhttp_settings_t g_settings;

static int
handle_on_method(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpParser*>(http);
  parser->set_field("method", std::string_view(at, length));
  return HPE_OK;
}

static int
handle_on_url(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpParser*>(http);
  parser->set_field("url", std::string_view(at, length));
  return HPE_OK;
}

static int
handle_on_version(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpParser*>(http);
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
  auto parser = static_cast<HttpParser*>(http);
  parser->handle();
  return HPE_OK;
}

static void
init()
{
  llhttp_settings_init(&g_settings);
  g_settings.on_method = handle_on_method;
  g_settings.on_url = handle_on_url;
  g_settings.on_version = handle_on_version;
  g_settings.on_body = handle_on_body;
  g_settings.on_message_complete = handle_on_message_complete;
}
void
HttpSession::reply(ScheduleRef sch, CoroutineRef co, const Message& data)
{
}

HttpParser::HttpParser(ScheduleRef sch, CoroutineRef co, ConnectionRef conn)
  : Parser(sch, co, conn)
{
  static std::once_flag init_once_flag;
  std::call_once(init_once_flag, init);
  llhttp_init(this, HTTP_REQUEST, &g_settings);

  request_ = std::make_shared<JsonMessage>();
}

void
HttpParser::on_data(std::string_view data)
{
  enum llhttp_errno err = llhttp_execute(this, data.data(), data.length());
  if (err != HPE_OK) {
    request_->clear();
    llhttp_reset(this);
  }
}

void
HttpParser::handle()
{
  std::string response_name;
  const auto& request_body = request_->get_body();
  response_name += request_body.get_string("method");
  response_name += " ";
  response_name += request_body.get_string("url");

  try {
    Environment env;
    env.sch = sch_;
    env.co = co_;
    env.session = session_;

    auto response = Context::get_instance().message_builder->create(
      env, response_name, request_);

    request_ = std::make_shared<JsonMessage>();

    handle_success(response);

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
HttpParser::set_field(std::string_view name, std::string_view value)
{
  auto& request_body = request_->get_body();
  request_body.set_value(name, value);
}

void
HttpParser::handle_error(llhttp_status_t status)
{
  std::string response;
  const auto& request_body = request_->get_body();
  response += "HTTP/";
  response += request_body.get_value("version", "1.1");
  response += " ";
  response += std::to_string(status);
  response += " ";
  response += llhttp_status_name(status);
  response += "\r\n\r\n";

  conn_.send(response);
}

void
HttpParser::handle_success(MessagePtr message)
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

}
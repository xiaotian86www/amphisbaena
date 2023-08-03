
#include <cstdlib>
#include <llhttp.h>
#include <memory>
#include <rapidjson/document.h>

#include "builder.hpp"
#include "environment.hpp"
#include "http_message.hpp"
#include "http_server.hpp"
#include "log.hpp"
#include "message.hpp"
#include "session.hpp"

namespace amphisbaena {

struct content_t
{
  std::size_t capital;
  std::size_t length;
  const char* const_buffer;
  char buffer[];
};

static content_t*
init_content(std::size_t length)
{
  std::size_t captital = (length + 1024) / 1024 * 1024;
  content_t* content = static_cast<content_t*>(malloc(captital));
  if (!content)
    return content;
  content->capital = captital;
  content->length = length;
  content->const_buffer = nullptr;
  return content;
}

static content_t*
reinit_content(content_t* content, std::size_t length)
{
  if (length < content->capital) {
    content->length = length;
    content->const_buffer = nullptr;
    return content;
  }

  std::size_t captital = (length + 1024) / 1024 * 1024;
  content_t* new_content = static_cast<content_t*>(realloc(content, captital));
  if (!new_content)
    return new_content;
  new_content->capital = captital;
  new_content->length = length;
  new_content->const_buffer = nullptr;
  return new_content;
}

static void
deinit_content(content_t* content)
{
  free(content);
}

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
handle_on_header_field(llhttp_t* http, const char* at, size_t length)
{
  return HPE_OK;
}

static int
handle_on_header_value(llhttp_t* http, const char* at, size_t length)
{
  return HPE_OK;
}

static int
handle_on_header_field_complete(llhttp_t* http)
{
  return HPE_OK;
}

static int
handle_on_header_value_complete(llhttp_t* http)
{
  return HPE_OK;
}

static int
handle_on_headers_complete(llhttp_t* http)
{
  auto* parser = static_cast<HttpSession*>(http);
  content_t* content = static_cast<content_t*>(parser->data);
  assert(content);
  content = reinit_content(content, parser->content_length);
  if (!content)
    return HPE_INTERNAL;
  parser->data = content;
  return HPE_OK;
}

static int
handle_on_body(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpSession*>(http);
  // content_length为剩余content长度，加上length等于整个content长度
  // 当content分多次得到的时候，需要将之前的部分content缓存下来
  content_t* content = static_cast<content_t*>(parser->data);
  assert(content);

  if (content->length == length) {
    content->const_buffer = at;
  } else {
    memcpy(content->buffer + content->length - length - parser->content_length,
           at,
           length);
  }

  return HPE_OK;
}

static int
handle_on_message_complete(llhttp_t* http)
{
  auto parser = static_cast<HttpSession*>(http);
  auto content = static_cast<content_t*>(parser->data);
  try {
    if (content->length > 0) {
      parser->set_body(
        { content->const_buffer ? content->const_buffer : content->buffer,
          content->length });
    }
    parser->do_recv();
    return HPE_OK;
  } catch (...) {
    return HPE_INTERNAL;
  }
}

HttpSession::HttpSession(HttpServer* server,
                         ScheduleRef sch,
                         CoroutineRef co,
                         ConnectionRef conn)
  : server_(server)
  , sch_(sch)
  , co_(co)
  , conn_(conn)
  , request_(amphisbaena::MessageFactory::create("Http"))
{
  llhttp_init(this, HTTP_REQUEST, &server->settings);
  llhttp_t::data = init_content(0);
  env_.sch = sch_;
  env_.co = co_;
}

HttpSession::~HttpSession()
{
  auto* content = static_cast<content_t*>(llhttp_t::data);
  deinit_content(content);
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
HttpSession::on_recv(std::string_view data)
{
  enum llhttp_errno err = llhttp_execute(this, data.data(), data.length());
  if (err != HPE_OK) {
    reset();
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

  env_.down = shared_from_this();

  try {
    response = MessageBuilder::create(env_, response_name, request_);

    auto response_head = response->get_head();

    response_head->set_value("code", HTTP_STATUS_OK);
    response_head->set_value("version", request_head->get_string("version"));

  } catch (...) {
    request_ = amphisbaena::MessageFactory::create("Http");

    response = handle_error(request_head->get_string("version"));
  }

  send(response);
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

MessagePtr
HttpSession::handle_error(std::string_view version)
{
  auto response = amphisbaena::MessageFactory::create("Http");
  auto response_head = response->get_head();

  try {
    throw;
  } catch (const NotFoundException& ec) {
    response_head->set_value("code", HTTP_STATUS_NOT_FOUND);
  } catch (const NoKeyException& ec) {
    response_head->set_value("code", HTTP_STATUS_BAD_REQUEST);
  } catch (const TypeExecption& ec) {
    response_head->set_value("code", HTTP_STATUS_BAD_REQUEST);
  } catch (const UnknownKeyException& ec) {
    response_head->set_value("code", HTTP_STATUS_BAD_REQUEST);
  } catch (const Exception& ec) {
    response_head->set_value("code", HTTP_STATUS_INTERNAL_SERVER_ERROR);
  }

  response_head->set_value("version", version);

  return response;
}

void
HttpSession::reset()
{
  llhttp_reset(this);
  request_ = amphisbaena::MessageFactory::create("Http");
}

HttpServer::HttpServer(std::shared_ptr<ServerFactory> server_factory)
  : server_(std::move(server_factory->create(this)))
{
  LOG_INFO("HttpServer create");

  llhttp_settings_init(&settings);
  settings.on_method = handle_on_method;
  settings.on_url = handle_on_url;
  settings.on_version = handle_on_version;
  settings.on_body = handle_on_body;
  settings.on_header_field = handle_on_header_field;
  settings.on_header_value = handle_on_header_value;
  settings.on_header_field_complete = handle_on_header_field_complete;
  settings.on_header_value_complete = handle_on_header_value_complete;
  settings.on_headers_complete = handle_on_headers_complete;
  settings.on_message_complete = handle_on_message_complete;
}

HttpServer::~HttpServer()
{
  LOG_INFO("HttpServer destroy");
}

void
HttpServer::on_recv(ScheduleRef sch,
                    CoroutineRef co,
                    ConnectionPtr conn,
                    std::string_view data)
{
  HttpSessionPtr session;
  if (auto iter = sessions_.find(conn); iter == sessions_.end()) {
    session = std::make_shared<HttpSession>(this, sch, co, conn);
    sessions_.insert_or_assign(conn, session);
  } else {
    session = iter->second;
  }

  session->on_recv(data);
}
}
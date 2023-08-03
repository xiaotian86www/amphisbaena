
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

// static content_t*
// init_content(std::size_t length)
// {
//   std::size_t captital = (length + 1024) / 1024 * 1024;
//   auto content = static_cast<content_t*>(malloc(captital));
//   if (!content)
//     return content;
//   content->capital = captital;
//   content->length = length;
//   return content;
// }

// static content_t*
// reinit_content(content_t* content, std::size_t length)
// {
//   if (length < content->capital) {
//     content->length = length;
//     return content;
//   }

//   std::size_t captital = (length + 1024) / 1024 * 1024;
//   auto new_content = static_cast<content_t*>(realloc(content, captital));
//   if (!new_content)
//     return new_content;
//   new_content->capital = captital;
//   new_content->length = length;
//   return new_content;
// }

// static void
// deinit_content(content_t* content)
// {
//   free(content);
// }

llhttp_settings_t HttpSession::settings_ = {
  .on_message_begin = nullptr,
  .on_url = handle_on_url,
  .on_status = nullptr,
  .on_method = handle_on_method,
  .on_version = handle_on_version,
  .on_header_field = nullptr,
  .on_header_value = nullptr,
  .on_chunk_extension_name = nullptr,
  .on_chunk_extension_value = nullptr,
  .on_headers_complete = handle_on_headers_complete,
  .on_body = handle_on_body,
  .on_message_complete = handle_on_message_complete,
  .on_url_complete = nullptr,
  .on_status_complete = nullptr,
  .on_method_complete = nullptr,
  .on_version_complete = nullptr,
  .on_header_field_complete = nullptr,
  .on_header_value_complete = nullptr,
  .on_chunk_extension_name_complete = nullptr,
  .on_chunk_extension_value_complete = nullptr,
  .on_chunk_header = nullptr,
  .on_chunk_complete = nullptr,
  .on_reset = handle_on_reset
};

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
  llhttp_init(this, HTTP_REQUEST, &settings_);
  env_.sch = sch_;
  env_.co = co_;
}

HttpSession::~HttpSession() {}

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
    llhttp_reset(this);
    request_ = amphisbaena::MessageFactory::create("Http");
  }
}

int
HttpSession::handle_on_method(llhttp_t* http, const char* at, size_t length)
{
  auto parser = static_cast<HttpSession*>(http);
  parser->request_->get_head()->set_value("method", { at, length });
  return HPE_OK;
}

int
HttpSession::handle_on_url(llhttp_t* http, const char* at, size_t length)
{
  auto parser = static_cast<HttpSession*>(http);
  parser->request_->get_head()->set_value("url", { at, length });
  return HPE_OK;
}

int
HttpSession::handle_on_version(llhttp_t* http, const char* at, size_t length)
{
  auto parser = static_cast<HttpSession*>(http);
  parser->request_->get_head()->set_value("version", { at, length });
  return HPE_OK;
}

int
HttpSession::handle_on_headers_complete(llhttp_t* http)
{
  auto parser = static_cast<HttpSession*>(http);
  parser->content_.clear();
  parser->content_.reserve(parser->content_length);
  return HPE_OK;
}

int
HttpSession::handle_on_body(llhttp_t* http, const char* at, size_t length)
{
  auto parser = static_cast<HttpSession*>(http);
  // content_length为剩余content长度，加上length等于整个content长度
  // 当content分多次得到的时候，需要将之前的部分content缓存下来
  if (parser->content_.size() == length) {
    auto request_body = parser->request_->get_body();
    static_cast<JsonObject*>(request_body.get())->from_string({ at, length });
  } else {
    parser->content_.append(at, length);
    if (http->content_length == 0) {
      auto request_body = parser->request_->get_body();
      static_cast<JsonObject*>(request_body.get())
        ->from_string(parser->content_);
    }
  }

  return HPE_OK;
}

int
HttpSession::handle_on_message_complete(llhttp_t* http)
{
  auto parser = static_cast<HttpSession*>(http);

  std::string response_name;
  auto request_head = parser->request_->get_head();
  response_name += request_head->get_string("method");
  response_name += " ";
  response_name += request_head->get_string("url");

  MessagePtr response;

  parser->env_.down = parser->shared_from_this();

  try {
    response =
      MessageBuilder::create(parser->env_, response_name, parser->request_);

    auto response_head = response->get_head();

    response_head->set_value("code", HTTP_STATUS_OK);
    response_head->set_value("version", request_head->get_string("version"));

  } catch (...) {
    response = parser->handle_error(request_head->get_string("version"));
  }

  parser->send(response);

  return HPE_OK;
}

int
HttpSession::handle_on_reset(llhttp_t* http)
{
  auto parser = static_cast<HttpSession*>(http);
  parser->request_ = amphisbaena::MessageFactory::create("Http");

  return HPE_OK;
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

HttpServer::HttpServer(std::shared_ptr<ServerFactory> server_factory)
  : server_(std::move(server_factory->create(this)))
{
  LOG_INFO("HttpServer create");
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

#include <cstdlib>
#include <llhttp.h>
#include <memory>

#include "builder.hpp"
#include "environment.hpp"
#include "http_parser.hpp"
#include "log.hpp"
#include "message.hpp"
#include "session.hpp"

namespace amphisbaena {

llhttp_settings_t HttpSession::settings_ = {
  .on_message_begin = handle_on_message_begin,
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
  .on_reset = nullptr
};

HttpSession::HttpSession(Session::MessageHandler* message_handle,
                         ScheduleRef sch,
                         CoroutineRef co,
                         ConnectionRef conn)
  : message_handle_(message_handle)
  , sch_(sch)
  , co_(co)
  , conn_(conn)
{
  llhttp_init(this, HTTP_REQUEST, &settings_);
  env_.sch = sch_;
  env_.co = co_;
}

HttpSession::~HttpSession() {}

void
HttpSession::send(MessagePtr message)
{
  auto head = message->get_head();
  auto body = message->get_body();
  const auto& body_str = body.get()->to_string();

  std::string response = fmt::format(
    "HTTP/{} {} {}\r\n"
    "Content-Type: application/json; charset=utf-8\r\n"
    "Content-Length: {}\r\n\r\n"
    "{}",
    head->get_string("version"),
    head->get_int("code"),
    llhttp_status_name(static_cast<llhttp_status_t>(head->get_int("code"))),
    body_str.length(),
    body_str);

  conn_.send(response);
}

void
HttpSession::on_recv(std::string_view data)
{
  enum llhttp_errno err = llhttp_execute(this, data.data(), data.length());
  if (err != HPE_OK) {
    llhttp_reset(this);
  }
}

int
HttpSession::handle_on_message_begin(llhttp_t* http)
{
  auto parser = static_cast<HttpSession*>(http);
  parser->request_ = amphisbaena::MessageFactory::create("Http");
  return HPE_OK;
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
    request_body.get()->from_string({ at, length });
  } else {
    parser->content_.append(at, length);
    if (http->content_length == 0) {
      auto request_body = parser->request_->get_body();
      request_body.get()->from_string(parser->content_);
    }
  }

  return HPE_OK;
}

int
HttpSession::handle_on_message_complete(llhttp_t* http)
{
  auto parser = static_cast<HttpSession*>(http);

  parser->message_handle_->on_recv(
    parser->sch_, parser->co_, parser->shared_from_this(), parser->request_);

  return HPE_OK;
}

MessagePtr
HttpSession::handle_error(std::string_view version)
{
  auto response = amphisbaena::MessageFactory::create("Http");
  auto response_head = response->get_head();
  auto response_body = response->get_body();

  try {
    throw;
  } catch (const NotFoundException& ec) {
    response_head->set_value("code", HTTP_STATUS_NOT_FOUND);
    response_body->set_value("description", ec.what());
  } catch (const NoKeyException& ec) {
    response_head->set_value("code", HTTP_STATUS_BAD_REQUEST);
    response_body->set_value("description", ec.what());
  } catch (const TypeExecption& ec) {
    response_head->set_value("code", HTTP_STATUS_BAD_REQUEST);
    response_body->set_value("description", ec.what());
  } catch (const UnknownKeyException& ec) {
    response_head->set_value("code", HTTP_STATUS_BAD_REQUEST);
    response_body->set_value("description", ec.what());
  } catch (const Exception& ec) {
    response_head->set_value("code", HTTP_STATUS_INTERNAL_SERVER_ERROR);
    response_body->set_value("description", ec.what());
  }

  response_head->set_value("version", version);

  return response;
}

HttpServer::HttpServer(std::shared_ptr<ServerFactory> server_factory,
                       Session::MessageHandler* message_handler)
  : message_handler_(message_handler)
  , server_(std::move(server_factory->create(this)))
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
    session = std::make_shared<HttpSession>(message_handler_, sch, co, conn);
    sessions_.insert_or_assign(conn, session);
  } else {
    session = iter->second;
  }

  session->on_recv(data);
}
}
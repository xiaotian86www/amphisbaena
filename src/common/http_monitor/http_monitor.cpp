
#include "http_monitor.hpp"
#include "application.hpp"
#include "llhttp.h"
#include "log.hpp"
#include "message.hpp"
#include "util.hpp"

namespace amphisbaena {

HttpMonitor::HttpMonitor(std::shared_ptr<ServerFactory> server_factory)
  : http_parser_(new HttpServer(server_factory, *this))
{
}

void
HttpMonitor::on_recv(ScheduleRef /* sch */,
                     CoroutineRef /* co */,
                     SessionPtr session,
                     MessagePtr message)
{
  auto request_head = message->get_head();
  auto request_body = message->get_body();

  std::string_view url = request_head->get_string("url");
  std::string_view method = request_head->get_string("method");

  LOG_DEBUG(
    "method: {}, url: {}, body: {}", method, url, request_body->to_string());

  auto url_elements = util::split(url, "/");

  auto url_element = url_elements.begin();
  if ((url_element != url_elements.end() && *url_element == "v1") &&
      (++url_element != url_elements.end() && *url_element == "api") &&
      (++url_element != url_elements.end() && *url_element == "plugins")) {

    auto id =
      ++url_element != url_elements.end() ? *url_element : std::string_view();

    try {
      if (method == "POST") {
        if (!id.empty()) {
          on_load(session, id, request_body);
        } else {
          on_bad_request(session);
        }
      } else if (method == "DELETE") {
        if (!id.empty()) {
          on_unload_plugin(session, id);
        } else {
          on_bad_request(session);
        }
      } else if (method == "PUT") {
        if (!id.empty()) {
          on_reload(session, id, request_body);
        } else {
          on_bad_request(session);
        }
      } else {
        // TODO 查询插件
      }
    } catch (const MessageException& ec) {
      on_bad_request(session);
    } catch (...) {
      
    }
  }
}

void
HttpMonitor::on_load(SessionPtr session,
                     std::string_view id,
                     ObjectPtr request_body)
{
  auto response = MessageFactory::create("http");
  auto path = request_body->get_string("path");
  if (Application::get_instance().load(id, path, {})) {
    auto response_data = init_response(response, HTTP_STATUS_OK, 0, "请求成功");

    response_data->set_value("id", id);
  } else {
    auto response_data =
      init_response(response, HTTP_STATUS_NOT_FOUND, 0, "插件找不到");

    response_data->set_value("id", id);
  }

  session->send(response);
}

void
HttpMonitor::on_reload(SessionPtr session,
                       std::string_view id,
                       ObjectPtr request_body)
{
  auto response = MessageFactory::create("http");
  auto path = request_body->get_string("path");
  if (Application::get_instance().reload(id, path, {})) {
    auto response_data = init_response(response, HTTP_STATUS_OK, 0, "请求成功");

    response_data->set_value("id", id);
  } else {
    auto response_data =
      init_response(response, HTTP_STATUS_NOT_FOUND, 0, "插件找不到");

    response_data->set_value("id", id);
  }

  session->send(response);
}

void
HttpMonitor::on_unload_plugin(SessionPtr session, std::string_view id)
{
  auto response = MessageFactory::create("http");
  if (Application::get_instance().unload(id)) {
    auto response_data = init_response(response, HTTP_STATUS_OK, 0, "请求成功");

    response_data->set_value("id", id);
  } else {
    auto response_data =
      init_response(response, HTTP_STATUS_NOT_FOUND, 0, "插件找不到");

    response_data->set_value("id", id);
  }

  session->send(response);
}

void
HttpMonitor::on_bad_request(SessionPtr session)
{
  auto response = MessageFactory::create("http");

  init_response(response, HTTP_STATUS_BAD_REQUEST, 0, "参数错误");

  session->send(response);
}

ObjectPtr
HttpMonitor::init_response(MessagePtr message,
                           int32_t http_code,
                           int32_t code,
                           std::string_view descprition)
{
  auto head = message->get_head();
  auto body = message->get_body();

  head->set_value("code", http_code);
  head->set_value("version", HTTP_VERSION);

  body->set_value("code", code);
  body->set_value("status", http_code);
  body->set_value("message", descprition);

  return body->get_or_set_object("body");
}
}
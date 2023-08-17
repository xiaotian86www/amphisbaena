
#include "http_monitor.hpp"
#include "application.hpp"
#include "log.hpp"

namespace amphisbaena {
HttpMonitor::HttpMonitor(std::shared_ptr<ServerFactory> server_factory)
  : http_parser_(new HttpServer(server_factory, this))
{
}

void
HttpMonitor::on_recv(ScheduleRef sch,
                     CoroutineRef co,
                     SessionPtr session,
                     MessagePtr message)
{
  auto request_head = message->get_head();
  auto request_body = message->get_body();

  std::string_view url = request_head->get_string("url");
  std::string_view method = request_head->get_string("method");

  LOG_DEBUG(
    "method: {}, url: {}, body: {}", method, url, request_body->to_string());

  if (url.rfind("/v1/admin/plugins", 0) == 0) {
    if (method == "POST") {
      // TODO 加载插件
      Application::get_instance().load("", "", {});
    } else if (method == "PUT") {
      // TODO 更新插件
      Application::get_instance().reload("", "", {});
    }
  }
}
}
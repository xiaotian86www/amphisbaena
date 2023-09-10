
#include "common/http_parser/http_parser.hpp"
#include "message.hpp"

namespace amphisbaena {
class HttpMonitor : public Session::MessageHandler
{
private:
  constexpr static const char* HTTP_VERSION = "1.1";

public:
  HttpMonitor(std::shared_ptr<ServerFactory> server_factory);

public:
  void on_recv(ScheduleRef sch,
               CoroutineRef co,
               SessionPtr session,
               MessagePtr message) override;

private:
  void on_load(SessionPtr session, std::string_view id, ObjectPtr request_body);

  void on_reload(SessionPtr session, std::string_view id, ObjectPtr request_body);

  void on_unload_plugin(SessionPtr session, std::string_view id);

  void on_bad_request(SessionPtr session);

  ObjectPtr init_response(MessagePtr message,
                     int32_t http_code,
                     int32_t code,
                     std::string_view descprition);

private:
  std::unique_ptr<HttpServer> http_parser_;
};
}
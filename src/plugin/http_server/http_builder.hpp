#include <memory>

#include "common/http_parser/http_parser.hpp"

namespace amphisbaena {
class HttpBuilder : public Session::MessageHandler
{
public:
  HttpBuilder(std::shared_ptr<ServerFactory> server_factory);

public:
  void on_recv(ScheduleRef sch,
               CoroutineRef co,
               SessionPtr session,
               MessagePtr message) override;

private:
  MessagePtr handle_error(std::string_view version);

private:
  std::unique_ptr<HttpServer> http_parser_;
};
}
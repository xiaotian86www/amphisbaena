
#include "common/http_parser/http_parser.hpp"

namespace amphisbaena {
class HttpMonitor : public Session::MessageHandler
{
public:
  HttpMonitor(std::shared_ptr<ServerFactory> server_factory);

public:
  void on_recv(ScheduleRef sch,
               CoroutineRef co,
               SessionPtr session,
               MessagePtr message) override;

private:
  std::unique_ptr<HttpServer> http_parser_;
};
}
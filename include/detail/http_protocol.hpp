#include "protocol.hpp"

#include <llhttp.h>
#include <string_view>

namespace translator {
struct HttpRequest
{
  std::string method;
  std::string url;
};

struct HttpResponse
{};

class HttpProtocol
  : public Protocol
  , public llhttp_t
{
public:
  HttpProtocol();
  ~HttpProtocol() override = default;

public:
  void on_data(ScheduleRef sch,
               CoroutineRef co,
               std::shared_ptr<Connection> conn,
               std::string_view data) override;

public:
  HttpRequest request;
};

class HttpProtocolFactory : public ProtocolFactory
{
public:
  std::unique_ptr<Protocol> create() override;
};
}
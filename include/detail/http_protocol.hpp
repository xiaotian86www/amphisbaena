#include "protocol.hpp"

#include <llhttp.h>
#include <string_view>

namespace translator {
struct HttpRequest
{
  std::string_view method;
  std::string_view url;
};

struct HttpResponse
{};

class HttpProtocol : public Protocol
{
public:
  HttpProtocol();
  ~HttpProtocol() override = default;

public:
  void on_data(ScheduleRef sch,
               CoroutineRef co,
               std::shared_ptr<Socket> sock,
               std::string_view data) override;

private:
  llhttp_t parser_;
  llhttp_settings_t settings_;
};

class HttpProtocolFactory : public ProtocolFactory
{
public:
  std::unique_ptr<Protocol> create() override;
};
}
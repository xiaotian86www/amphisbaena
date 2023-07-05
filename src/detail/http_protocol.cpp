#include "detail/http_protocol.hpp"
#include "server.hpp"
#include <llhttp.h>
#include <memory>

namespace translator {

HttpProtocol::HttpProtocol()
{
  llhttp_settings_init(&settings_);
}

void
HttpProtocol::on_data(std::shared_ptr<Socket> sock,
                      CoroutineRef co,
                      std::string_view data)
{
}

std::unique_ptr<Protocol>
HttpProtocolFactory::create()
{
  return std::unique_ptr<HttpProtocol>();
}
}
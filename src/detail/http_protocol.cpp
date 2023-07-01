#include "detail/http_protocol.hpp"
#include <llhttp.h>

namespace translator {

HttpProtocol::HttpProtocol()
{
    llhttp_settings_init(&settings_);
}

void
HttpProtocol::on_data(std::shared_ptr<Socket> sock,
                      std::shared_ptr<Coroutine> co,
                      std::string_view data)
{

}
}
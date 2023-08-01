#include "http_to_fix_builder.hpp"
#include "builder.hpp"
#include "environment.hpp"
#include "message.hpp"
#include "log.hpp"

namespace amphisbaena {
namespace builder {

HttpToFixBuilder::HttpToFixBuilder()
{
  LOG_INFO("HttpToFixBuilder create");
}

HttpToFixBuilder::~HttpToFixBuilder()
{
  LOG_INFO("HttpToFixBuilder destroy");
}

MessagePtr
HttpToFixBuilder::create(Environment& env, MessagePtr request)
{
  auto fix_request = MessageFactory::create("Fix");
  fix_request->get_body()->copy_from(request->get_body());

  auto fix_response = MessageBuilder::create(env, "fix", fix_request);

  auto json_response = MessageFactory::create("Json");
  json_response->get_body()->copy_from(fix_response->get_body());

  return json_response;
}

std::string_view
HttpToFixBuilder::name() const
{
  return "GET /";
}

}
}

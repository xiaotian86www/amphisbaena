#include "http_to_fix_builder.hpp"
#include "builder.hpp"
#include "environment.hpp"
#include "log.hpp"
#include "message.hpp"

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
  auto http_request_body = request->get_body();

  auto fix_request = MessageFactory::create("fix");
  fix_request->get_head()->copy_from(http_request_body->get_object("head"));
  fix_request->get_body()->copy_from(http_request_body->get_object("body"));

  auto fix_response = MessageBuilder::create(env, "fix", fix_request);

  auto http_response = MessageFactory::create("Http");
  auto http_response_body = http_response->get_body();
  http_response_body->get_or_set_object("head")->copy_from(
    fix_response->get_head());
  http_response_body->get_or_set_object("body")->copy_from(
    fix_response->get_body());

  return http_response;
}

std::string_view
HttpToFixBuilder::name() const
{
  return "GET /";
}

}
}

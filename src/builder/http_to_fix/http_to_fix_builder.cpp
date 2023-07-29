#include "http_to_fix_builder.hpp"
#include "builder.hpp"
#include "environment.hpp"
#include "message.hpp"

namespace translator {
namespace builder {
MessagePtr
HttpToFixBuilder::create(Environment& env, MessagePtr request)
{
  auto fix_request = MessageFactory::create("Fix");
  fix_request->get_body()->copy_from(request->get_body());

  fix_request->get_body()->get_string("SenderCompID");

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

extern "C"
{
  translator::MessageBuilder* get_func()
  {
    return new translator::builder::HttpToFixBuilder();
  }

  const char* get_name()
  {
    return "GET /";
  }
}
#include "builder.hpp"
#include "environment.hpp"
#include "message.hpp"

namespace translator {
namespace builder {
MessagePtr
HttpToFixBuilder::operator()(Environment& env, MessagePtr request)
{
  auto fix_request = MessageFactory::create("Fix");
  fix_request->get_body()->copy_from(request->get_body());

  fix_request->get_body()->get_string("SenderCompID");

  auto fix_response = env.builder->create(env, "fix", fix_request);

  auto json_response = MessageFactory::create("Json");
  json_response->get_body()->copy_from(fix_response->get_body());

  return json_response;
}
}
}

extern "C"
{
  void* get_func()
  {
    return new translator::MessageBuilder::ctor_function(
      translator::builder::HttpToFixBuilder());
  }

  const char* get_name()
  {
    return "GET /";
  }
}
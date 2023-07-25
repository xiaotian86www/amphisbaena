#include "builder.hpp"
#include "environment.hpp"
#include "message.hpp"

namespace translator {
namespace builder {
class HttpToFixBuilder
{
public:
  MessagePtr operator()(Environment& env, MessagePtr request)
  {
    auto fix_request = MessageFactory::create(MessageType::kFix);
    fix_request->get_body()->copy_from(request->get_body());

    auto fix_response = env.builder->create(env, "fix", fix_request);

    auto json_response = MessageFactory::create(MessageType::kJson);
    json_response->get_body()->copy_from(fix_response->get_body());

    return json_response;
  }
};
}
}
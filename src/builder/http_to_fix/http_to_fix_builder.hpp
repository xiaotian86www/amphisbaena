
#include "builder.hpp"
#include "environment.hpp"
#include "message.hpp"

namespace translator {
namespace builder {
class HttpToFixBuilder : public MessageBuilder
{
public:
  HttpToFixBuilder();
  
  ~HttpToFixBuilder() override;

public:
  MessagePtr create(Environment& env, MessagePtr request) override;

  std::string_view name() const override;
};
}
}
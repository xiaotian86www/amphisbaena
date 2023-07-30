
#include <memory>

#include "builder.hpp"
#include "environment.hpp"
#include "impl/json_message.hpp"
#include "message.hpp"

class MessageBuilder1 : public translator::MessageBuilder
{
public:
  translator::MessagePtr create(translator::Environment&,
                                translator::MessagePtr) override
  {
    auto message = std::make_shared<translator::JsonMessage>();
    message->get_body()->set_value("Field", 1);
    return message;
  }

  std::string_view name() const override { return "Message"; }
};

extern "C"
{
  void init2(int argc, const char** argv)
  {
  }
}
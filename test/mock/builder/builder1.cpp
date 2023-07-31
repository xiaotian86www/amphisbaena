
#include <memory>

#include "builder.hpp"
#include "environment.hpp"
#include "impl/json_message.hpp"
#include "message.hpp"

class MessageBuilder1 : public amphisbaena::MessageBuilder
{
public:
  amphisbaena::MessagePtr create(amphisbaena::Environment&,
                                amphisbaena::MessagePtr) override
  {
    auto message = std::make_shared<amphisbaena::JsonMessage>();
    message->get_body()->set_value("Field", 1);
    return message;
  }

  std::string_view name() const override { return "Message"; }
};

extern "C"
{
  void init(int argc, const char** argv)
  {
    amphisbaena::MessageBuilder::registe("Message",
                                        std::make_shared<MessageBuilder1>());
  }
}
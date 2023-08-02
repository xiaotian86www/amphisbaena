
#include <memory>

#include "builder.hpp"
#include "environment.hpp"
#include "message.hpp"
#include "plugin/http_server/json_message.hpp"

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
  void init2(int argc, const char** argv) {}
}
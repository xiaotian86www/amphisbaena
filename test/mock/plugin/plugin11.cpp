
#include <memory>

#include "builder.hpp"
#include "common/http_message/http_message.hpp"
#include "environment.hpp"
#include "message.hpp"

class MessageBuilder1 : public amphisbaena::MessageBuilder
{
public:
  amphisbaena::MessagePtr create(amphisbaena::Environment&,
                                 amphisbaena::MessagePtr) override
  {
    auto message = std::make_shared<amphisbaena::HttpMessage>();
    message->get_body()->set_value("Field", 1);
    return message;
  }

  std::string_view name() const override { return "Message"; }
};

extern "C"
{
  void init2(int argc, const char** argv) {}
}

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
    message->get_body()->set_value("Field", 2);
    return message;
  }

  std::string_view name() const override { return "Message"; }
};

static std::shared_ptr<amphisbaena::MessageBuilder> builder;

extern "C"
{
  void init(int /* argc */, const char** /* argv */)
  {
    builder = std::make_shared<MessageBuilder1>();
    amphisbaena::MessageBuilder::registe(builder);
  }

  void deinit()
  {
    amphisbaena::MessageBuilder::unregiste(builder);
    builder.reset();
  }
}
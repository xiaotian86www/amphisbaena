
#include <memory>

#include "builder.hpp"
#include "common/http_message/http_message.hpp"
#include "environment.hpp"
#include "loader.hpp"
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

class Plugin1Loader : public amphisbaena::Loader
{
protected:
  void do_init(int /* argc */, const char* const* /* argv */) override
  {
    builder = std::make_shared<MessageBuilder1>();
    amphisbaena::MessageBuilder::registe(builder);
  }

  void do_deinit() override
  {
    amphisbaena::MessageBuilder::unregiste(builder);
    builder.reset();
  }

private:
  std::shared_ptr<amphisbaena::MessageBuilder> builder;
};

AMP_REGISTE_PLUGIN_LOADER(Plugin1Loader)
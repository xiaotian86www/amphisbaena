#include "builder.hpp"
#include "fix_builder.hpp"
#include "fix_client.hpp"
#include "fix_message.hpp"
#include "message.hpp"

static std::shared_ptr<amphisbaena::MessageBuilder> builder;
static std::shared_ptr<amphisbaena::MessageFactory> factory;

extern "C"
{
  void init(int argc, const char* const* argv)
  {
    if (argc < 2)
      throw std::invalid_argument("Usage: " + std::string(argv[0]));
    
    amphisbaena::FixClientFactory client_factory(argv[1]);
    builder = std::make_shared<amphisbaena::FixBuilder>(client_factory);
    factory = std::make_shared<amphisbaena::FixMessageFactory>();

    amphisbaena::MessageBuilder::registe(builder);
    amphisbaena::MessageFactory::registe(factory);
  }

  void deinit()
  {
    amphisbaena::MessageBuilder::unregiste(builder);
    amphisbaena::MessageFactory::unregiste(factory);

    builder.reset();
    factory.reset();
  }
}
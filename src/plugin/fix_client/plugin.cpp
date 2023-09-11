#include "builder.hpp"
#include "fix_builder.hpp"
#include "fix_client.hpp"
#include "fix_message.hpp"
#include "loader.hpp"
#include "message.hpp"

namespace amphisbaena {
class FixClientLoader : public amphisbaena::Loader
{
public:
  void do_init(int argc, const char* const* argv) override
  {
    if (argc < 2)
      throw std::invalid_argument("Usage: " + std::string(argv[0]));

    amphisbaena::FixClientFactory client_factory(argv[1]);
    builder_ = std::make_shared<amphisbaena::FixBuilder>(client_factory);
    factory_ = std::make_shared<amphisbaena::FixMessageFactory>();

    amphisbaena::MessageBuilder::registe(builder_);
    amphisbaena::MessageFactory::registe(factory_);
  }

  void do_deinit() override
  {
    amphisbaena::MessageBuilder::unregiste(builder_);
    amphisbaena::MessageFactory::unregiste(factory_);

    builder_.reset();
    factory_.reset();
  }

private:
  std::shared_ptr<amphisbaena::MessageBuilder> builder_;
  std::shared_ptr<amphisbaena::MessageFactory> factory_;
};
}

AMP_REGISTE_PLUGIN_LOADER(amphisbaena::FixClientLoader)
#include "builder.hpp"
#include "http_to_fix_builder.hpp"
#include "loader.hpp"

namespace amphisbaena {
class HttpToFixLoader : public amphisbaena::Loader
{
public:
  void do_init(int /* argc */, const char* const* /* argv */) override
  {
    builder = std::make_shared<amphisbaena::builder::HttpToFixBuilder>();
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
}

AMP_REGISTE_PLUGIN_LOADER(amphisbaena::HttpToFixLoader)
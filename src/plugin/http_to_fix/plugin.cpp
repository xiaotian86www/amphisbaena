#include "builder.hpp"
#include "http_to_fix_builder.hpp"

static std::shared_ptr<amphisbaena::MessageBuilder> builder;

extern "C"
{
  void init(int argc, const char* const* argv)
  {
    builder = std::make_shared<amphisbaena::builder::HttpToFixBuilder>();
    amphisbaena::MessageBuilder::registe(builder);
  }

  void deinit()
  {
    amphisbaena::MessageBuilder::unregiste(builder);
    builder.reset();
  }
}
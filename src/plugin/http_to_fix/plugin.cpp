#include "http_to_fix_builder.hpp"

extern "C"
{
  void init(int argc, const char* const* argv)
  {
    amphisbaena::MessageBuilder::registe(
      "GET /", std::make_shared<amphisbaena::builder::HttpToFixBuilder>());
  }

  void deinit()
  {
    amphisbaena::MessageBuilder::unregiste("GET /");
  }
}
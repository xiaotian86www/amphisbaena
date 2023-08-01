#include "builder.hpp"
#include "fix_builder.hpp"
#include "fix_client.hpp"
#include "fix_message.hpp"
#include "message.hpp"

extern "C"
{
  void init(int argc, const char* const* argv)
  {
    if (argc < 2)
      throw std::invalid_argument("Usage: " + std::string(argv[0]));

    amphisbaena::FixClientFactory client_factory(argv[1]);

    amphisbaena::MessageBuilder::registe(
      "Fix", std::make_shared<amphisbaena::FixBuilder>(client_factory));

    amphisbaena::MessageFactory::registe(
      "Fix", [] { return std::make_shared<amphisbaena::FixMessage>(); });
  }

  void deinit()
  {
    // TODO 注册时是覆盖操作，当多次注册后，取消注册动作要防止取消了别的插件注册的句柄
    amphisbaena::MessageBuilder::unregiste("Fix");
    amphisbaena::MessageFactory::unregiste("Fix");
  }
}
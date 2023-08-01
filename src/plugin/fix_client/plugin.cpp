#include "fix_builder.hpp"
#include "fix_client.hpp"
#include "fix_message.hpp"

extern "C"
{
  void init(int argc, const char** argv)
  {
    if (argc < 2)
      throw std::invalid_argument("Usage: " + std::string(argv[0]));

    amphisbaena::FixClientFactory client_factory(argv[1]);

    amphisbaena::MessageBuilder::registe(
      "Fix", std::make_shared<amphisbaena::FixBuilder>(client_factory));

    amphisbaena::MessageFactory::registe(
      "Fix", [] { return std::make_shared<amphisbaena::FixMessage>(); });
  }
}
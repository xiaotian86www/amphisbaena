
#include <memory>
#include <thread>

#include "builder.hpp"
#include "common/http_parser/http_parser.hpp"
#include "common/uds_server/uds_server.hpp"
#include "http_message.hpp"
#include "message.hpp"

static std::shared_ptr<amphisbaena::MessageFactory> factory;
static boost::asio::io_service ios;
static std::thread th;

extern "C"
{
  void init(int argc, const char* const* argv)
  {
    if (argc < 2)
      throw std::invalid_argument("Usage: " + std::string(argv[0]));

    factory = std::make_shared<amphisbaena::HttpMessageFactory>();

    amphisbaena::MessageFactory::registe(factory);

    auto sch = std::make_shared<amphisbaena::Schedule>(ios);
    auto server_factory =
      std::make_shared<amphisbaena::UDSServerFactory>(ios, sch, argv[1]);
    auto http_server =
      std::make_shared<amphisbaena::HttpServer>(server_factory);

    th = std::thread([sch, http_server] { ios.run(); });
  }

  void deinit()
  {
    ios.stop();
    th.join();

    amphisbaena::MessageFactory::unregiste(factory);

    factory.reset();
  }
}

#include <boost/asio/io_service.hpp>
#include <memory>
#include <thread>

#include "builder.hpp"
#include "common/http_message/http_message.hpp"
#include "common/http_parser/http_parser.hpp"
#include "common/uds_server/uds_server.hpp"
#include "http_builder.hpp"
#include "message.hpp"
#include "schedule.hpp"

static boost::asio::io_service ios;
static std::shared_ptr<amphisbaena::Schedule> schedule;
static std::shared_ptr<amphisbaena::MessageFactory> factory;
static std::shared_ptr<amphisbaena::HttpBuilder> builder;
static std::thread ios_thread;

extern "C"
{
  void init(int argc, const char* const* argv)
  {
    if (argc < 2)
      throw std::invalid_argument("Usage: " + std::string(argv[0]));

    schedule = std::make_shared<amphisbaena::Schedule>(ios);
    factory = std::make_shared<amphisbaena::HttpMessageFactory>();
    amphisbaena::MessageFactory::registe(factory);

    builder = std::make_shared<amphisbaena::HttpBuilder>(
      std::make_shared<amphisbaena::UdsServerFactory>(ios, schedule, argv[1]));

    ios_thread = std::thread([] { ios.run(); });
  }

  void deinit()
  {
    ios.stop();
    ios_thread.join();

    amphisbaena::MessageFactory::unregiste(factory);

    builder.reset();
    factory.reset();
    schedule.reset();
  }
}
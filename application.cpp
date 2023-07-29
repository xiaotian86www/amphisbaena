
#include <boost/asio/io_service.hpp>
#include <cstdlib>
#include <memory>
#include <signal.h>

#include "builder.hpp"
#include "impl/fix_message.hpp"
#include "impl/http_server.hpp"
#include "impl/json_message.hpp"
#include "impl/uds_server.hpp"
#include "log.hpp"
#include "message.hpp"
#include "schedule.hpp"

static boost::asio::io_service ios;

void
sigint_handler(int sig)
{
  if (sig == SIGINT) {
    if (!ios.stopped())
      ios.stop();
  }
}

void
exit_handler()
{
  LOG_INFO("Translator end");
}

int
main(int argc, char** argv)
{
  spdlog::set_level(spdlog::level::debug);

  LOG_INFO("Translator begin");
  auto sch = std::make_shared<translator::Schedule>(ios);
  translator::HttpServer http_server(
    std::make_unique<translator::UDSServer>(ios, sch, "server.sock"));
  translator::MessageFactory::registe(
    "Fix", [] { return std::make_shared<translator::FixMessage>(); });
  translator::MessageFactory::registe(
    "Json", [] { return std::make_shared<translator::JsonMessage>(); });

  signal(SIGINT, sigint_handler);
  atexit(exit_handler);

  ios.run();

  return 0;
}
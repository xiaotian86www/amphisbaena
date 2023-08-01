
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
  amphisbaena::MessageBuilder::unregiste();
  amphisbaena::MessageFactory::unregiste();

  LOG_INFO("amphisbaena end");
}

int
main(int argc, char** argv)
{
  spdlog::set_level(spdlog::level::debug);

  LOG_INFO("amphisbaena begin");
  auto sch = std::make_shared<amphisbaena::Schedule>(ios);
  amphisbaena::UDSServerFactory server_factory(ios, sch, "server.sock");
  amphisbaena::HttpServer http_server(server_factory);
  amphisbaena::MessageFactory::registe(
    "Fix", [] { return std::make_shared<amphisbaena::FixMessage>(); });
  amphisbaena::MessageFactory::registe(
    "Json", [] { return std::make_shared<amphisbaena::JsonMessage>(); });

  amphisbaena::Plugin::load("src/builder/fix_client/libfix_client.so",
                           { "../cfg/fix_client/tradeclient.cfg" });
  amphisbaena::Plugin::load("src/builder/http_to_fix/libhttp_to_fix.so");

  signal(SIGINT, sigint_handler);
  atexit(exit_handler);

  ios.run();

  return 0;
}
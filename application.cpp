
#include <boost/asio/io_service.hpp>
#include <cstdlib>
#include <memory>
#include <signal.h>

#include "log.hpp"
#include "message.hpp"
#include "plugin.hpp"
#include "schedule.hpp"

static boost::asio::io_service ios;
static boost::asio::io_service::work work(ios);

void
sigint_handler(int sig)
{
  if (sig == SIGINT) {
    if (!ios.stopped())
      ios.stop();
  }
}

int
main(int argc, char** argv)
{
  spdlog::set_level(spdlog::level::debug);

  LOG_INFO("Amphisbaena begin");

  {
    amphisbaena::Plugin fix_client_plugin(
      "src/plugin/fix_client/libfix_client.so.0",
      { "../cfg/fix_client/tradeclient.cfg" });
    amphisbaena::Plugin http_to_fix_plugin(
      "src/plugin/http_to_fix/libhttp_to_fix.so.0");
    amphisbaena::Plugin http_server_plugin(
      "src/plugin/http_server/libhttp_server.so.0", { "server.sock" });

    signal(SIGINT, sigint_handler);

    ios.run();
  }

  LOG_INFO("Amphisbaena end");
  return 0;
}
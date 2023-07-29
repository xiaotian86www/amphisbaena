
#include <boost/asio/io_service.hpp>
#include <memory>

#include "builder.hpp"
#include "impl/fix_message.hpp"
#include "impl/http_server.hpp"
#include "impl/json_message.hpp"
#include "impl/uds_server.hpp"
#include "message.hpp"
#include "schedule.hpp"

int
main(int argc, char** argv)
{
  boost::asio::io_service ios;
  auto sch = std::make_shared<translator::Schedule>(ios);
  translator::HttpServer http_server(
    std::make_unique<translator::UDSServer>(ios, sch, "server.sock"));
  translator::MessageFactory::registe(
    "Fix", [] { return std::make_shared<translator::FixMessage>(); });
  translator::MessageFactory::registe(
    "Json", [] { return std::make_shared<translator::JsonMessage>(); });


  ios.run();

  return 0;
}
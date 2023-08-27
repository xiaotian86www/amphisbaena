
#include <memory>

#include "common/uds_server/uds_server.hpp"
#include "fixture_server.hpp"
#include "tool/http_client.hpp"

INSTANTIATE_TEST_SUITE_P(
  Uds,
  Server,
  testing::Values(std::make_pair(
    [](boost::asio::io_service& ios,
       std::shared_ptr<amphisbaena::Schedule> sch,
       amphisbaena::Connection::MessageHandler& message_handler) {
      return std::make_unique<amphisbaena::UdsServer>(
        ios, sch, "server.sock", message_handler);
    },
    []() { return std::make_unique<HttpClient>("server.sock"); })));

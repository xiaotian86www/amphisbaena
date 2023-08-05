
#include <memory>

#include "common/tcp_server/tcp_server.hpp"
#include "fixture_server.hpp"
#include "tool/http_client.hpp"

INSTANTIATE_TEST_SUITE_P(
  Tcp,
  Server,
  testing::Values(std::make_pair(
    [](boost::asio::io_service& ios,
       std::shared_ptr<amphisbaena::Schedule> sch,
       amphisbaena::Server::MessageHandler* message_handler) {
      return std::make_unique<amphisbaena::TcpServer>(
        ios, sch, 32001, message_handler);
    },
    []() { return std::make_unique<HttpClient>("127.0.0.1", 32001); })));

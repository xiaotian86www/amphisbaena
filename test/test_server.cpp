#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "mock/mock_server.hpp"
#include "server.hpp"

TEST(server, get_server)
{
  auto server_pool = std::make_shared<translator::ServerPool>();

  auto server = std::make_shared<MockServer>();

  server_pool->add("server1", server);
  EXPECT_EQ(server_pool->get("server1"), server.get());
}

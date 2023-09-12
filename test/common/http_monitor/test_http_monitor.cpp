#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "common/http_message/http_message.hpp"
#include "common/http_monitor/http_monitor.hpp"
#include "mock_server.hpp"

class HttpMonitor : public testing::Test
{
public:
  HttpMonitor()
    : http_message_factory(std::make_shared<amphisbaena::HttpMessageFactory>())
    , server_factory(std::make_shared<MockServerFactory>())
    , http_monitor(std::make_shared<amphisbaena::HttpMonitor>(server_factory))
  {
    amphisbaena::MessageFactory::registe(http_message_factory);
  }

  ~HttpMonitor() { amphisbaena::MessageFactory::unregiste(); }

protected:
  std::shared_ptr<amphisbaena::MessageFactory> http_message_factory;
  std::shared_ptr<amphisbaena::ServerFactory> server_factory;
  std::shared_ptr<amphisbaena::HttpMonitor> http_monitor;
};

TEST_F(HttpMonitor, load) {
    
}
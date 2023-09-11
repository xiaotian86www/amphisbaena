
#include <boost/asio/io_service.hpp>
#include <memory>
#include <thread>

#include "builder.hpp"
#include "common/http_message/http_message.hpp"
#include "common/http_parser/http_parser.hpp"
#include "common/uds_server/uds_server.hpp"
#include "http_builder.hpp"
#include "loader.hpp"
#include "message.hpp"
#include "schedule.hpp"

namespace amphisbaena {
class HttpServerLoader : public amphisbaena::Loader
{
public:
  void do_init(int argc, const char* const* argv) override
  {
    if (argc < 2)
      throw std::invalid_argument("Usage: " + std::string(argv[0]));

    schedule_ = std::make_shared<amphisbaena::Schedule>(ios_);
    factory_ = std::make_shared<amphisbaena::HttpMessageFactory>();
    amphisbaena::MessageFactory::registe(factory_);

    builder_ = std::make_shared<amphisbaena::HttpBuilder>(
      std::make_shared<amphisbaena::UdsServerFactory>(
        ios_, schedule_, argv[1]));

    ios_thread_ = std::thread([this] { ios_.run(); });
  }

  void do_deinit() override
  {
    ios_.stop();
    ios_thread_.join();

    amphisbaena::MessageFactory::unregiste(factory_);

    builder_.reset();
    factory_.reset();
    schedule_.reset();
  }

private:
  boost::asio::io_service ios_;
  std::shared_ptr<amphisbaena::Schedule> schedule_;
  std::shared_ptr<amphisbaena::MessageFactory> factory_;
  std::shared_ptr<amphisbaena::HttpBuilder> builder_;
  std::thread ios_thread_;
};
}

AMP_REGISTE_PLUGIN_LOADER(amphisbaena::HttpServerLoader)
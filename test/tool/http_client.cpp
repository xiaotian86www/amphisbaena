
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>
#include <fmt/core.h>
#include <llhttp.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "http_client.hpp"

class UdsClient : public Client
{
public:
  UdsClient(const std::filesystem::path& path, Client::MessageHandler* handler)
    : Client(handler)
    , work_(ios_)
    , sock_(ios_)
  {
    sock_.connect(path.string());

    sock_.async_read_some(boost::asio::buffer(buffer_.data(), buffer_.size()),
                          std::bind(&UdsClient::do_recv,
                                    this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));

    th_ = std::thread([this] { ios_.run(); });
  }

  ~UdsClient()
  {
    boost::system::error_code ec;
    sock_.close(ec);

    ios_.stop();
    th_.join();
  }

public:
  void send(std::string_view data) override
  {
    sock_.write_some(boost::asio::const_buffer(data.data(), data.size()));
  }

private:
  void do_recv(boost::system::error_code ec, std::size_t size)
  {
    if (ec)
      return;

    handler_->on_recv({ buffer_.data(), size });

    sock_.async_read_some(boost::asio::buffer(buffer_.data(), buffer_.size()),
                          std::bind(&UdsClient::do_recv,
                                    this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));
  }

private:
  boost::asio::io_service ios_;
  boost::asio::io_service::work work_;
  boost::asio::local::stream_protocol::socket sock_;
  std::array<char, 1024> buffer_;
  std::thread th_;
};

class TcpClient : public Client
{
public:
  TcpClient(std::string_view host,
            uint16_t port,
            Client::MessageHandler* handler)
    : Client(handler)
    , work_(ios_)
    , sock_(ios_)
  {
    sock_.connect(boost::asio::ip::tcp::endpoint(
      boost::asio::ip::address::from_string(std::string(host)), port));

    sock_.async_read_some(boost::asio::buffer(buffer_.data(), buffer_.size()),
                          std::bind(&TcpClient::do_recv,
                                    this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));

    th_ = std::thread([this] { ios_.run(); });
  }

  ~TcpClient()
  {
    boost::system::error_code ec;
    sock_.close(ec);

    ios_.stop();
    th_.join();
  }

public:
  void send(std::string_view data) override
  {
    sock_.write_some(boost::asio::const_buffer(data.data(), data.size()));
  }

private:
  void do_recv(boost::system::error_code ec, std::size_t size)
  {
    if (ec)
      return;

    handler_->on_recv({ buffer_.data(), size });

    sock_.async_read_some(boost::asio::buffer(buffer_.data(), buffer_.size()),
                          std::bind(&TcpClient::do_recv,
                                    this,
                                    std::placeholders::_1,
                                    std::placeholders::_2));
  }

private:
  boost::asio::io_service ios_;
  boost::asio::io_service::work work_;
  boost::asio::ip::tcp::socket sock_;
  std::array<char, 1024> buffer_;
  std::thread th_;
};

llhttp_settings_t HttpClient::settings_ = { handle_on_message_begin,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            handle_on_body,
                                            handle_on_message_complete,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr,
                                            nullptr };

HttpClient::HttpClient(const std::filesystem::path& path)
  : client_(new UdsClient(path, this))
{
  llhttp_init(this, HTTP_RESPONSE, &settings_);
}

HttpClient::HttpClient(std::string_view host, uint16_t port)
  : client_(new TcpClient(host, port, this))
{
  llhttp_init(this, HTTP_RESPONSE, &settings_);
}

HttpClient::~HttpClient() {}

void
HttpClient::send(std::string_view version,
                 std::string_view method,
                 std::string_view path,
                 std::string_view body)
{
  std::string request =
    fmt::format("{} {} HTTP/{}\r\n"
                "Content-Type: application/json; charset=utf-8\r\n"
                "Content-Length: {}\r\n\r\n"
                "{}",
                method,
                path,
                version,
                body.size(),
                body);

  client_->send(request);
}

void
HttpClient::on_recv(std::string_view data)
{
  enum llhttp_errno err = llhttp_execute(this, data.data(), data.size());
  if (err != HPE_OK) {
    llhttp_reset(this);
  }
}

int
HttpClient::handle_on_message_begin(llhttp_t* http)
{
  auto parser = static_cast<HttpClient*>(http);
  parser->content_.clear();

  return HPE_OK;
}

int
HttpClient::handle_on_body(llhttp_t* http, const char* at, size_t length)
{
  auto parser = static_cast<HttpClient*>(http);
  // content_length为剩余content长度，加上length等于整个content长度
  // 当content分多次得到的时候，需要将之前的部分content缓存下来

  parser->content_.append(at, length);

  return HPE_OK;
}

int
HttpClient::handle_on_message_complete(llhttp_t* http)
{
  auto parser = static_cast<HttpClient*>(http);
  parser->on_recv(parser->status_code, parser->content_);
  return HPE_OK;
}

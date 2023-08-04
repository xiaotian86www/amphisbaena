
#include <boost/system/error_code.hpp>
#include <fmt/core.h>
#include <llhttp.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "http_client.hpp"

llhttp_settings_t HttpClient::settings_ = {
  .on_message_begin = handle_on_message_begin,
  .on_url = nullptr,
  .on_status = nullptr,
  .on_method = nullptr,
  .on_version = nullptr,
  .on_header_field = nullptr,
  .on_header_value = nullptr,
  .on_chunk_extension_name = nullptr,
  .on_chunk_extension_value = nullptr,
  .on_headers_complete = nullptr,
  .on_body = handle_on_body,
  .on_message_complete = handle_on_message_complete,
  .on_url_complete = nullptr,
  .on_status_complete = nullptr,
  .on_method_complete = nullptr,
  .on_version_complete = nullptr,
  .on_header_field_complete = nullptr,
  .on_header_value_complete = nullptr,
  .on_chunk_extension_name_complete = nullptr,
  .on_chunk_extension_value_complete = nullptr,
  .on_chunk_header = nullptr,
  .on_chunk_complete = nullptr,
  .on_reset = nullptr
};

HttpClient::HttpClient(const std::filesystem::path& path)
  : work_(ios_)
  , sock_(ios_)
{
  sock_.connect(path.string());

  llhttp_init(this, HTTP_RESPONSE, &settings_);

  sock_.async_read_some(boost::asio::buffer(buffer_.data(), buffer_.size()),
                        std::bind(&HttpClient::do_recv,
                                  this,
                                  std::placeholders::_1,
                                  std::placeholders::_2));

  th_ = std::thread([this] { ios_.run(); });
}

HttpClient::~HttpClient()
{
  boost::system::error_code ec;
  sock_.close(ec);

  ios_.stop();
  th_.join();
}

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

  sock_.write_some(boost::asio::const_buffer(request.data(), request.size()));
}

void
HttpClient::do_recv(boost::system::error_code ec, std::size_t size)
{
  if (ec)
    return;

  enum llhttp_errno err = llhttp_execute(this, buffer_.data(), size);
  if (err != HPE_OK) {
    llhttp_reset(this);
  }

  sock_.async_read_some(boost::asio::buffer(buffer_.data(), buffer_.size()),
                        std::bind(&HttpClient::do_recv,
                                  this,
                                  std::placeholders::_1,
                                  std::placeholders::_2));
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
  parser->on_recv(static_cast<llhttp_status>(parser->status_code),
                  parser->content_);
  return HPE_OK;
}

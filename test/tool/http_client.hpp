#pragma once

#include <array>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <filesystem>
#include <gmock/gmock.h>
#include <llhttp.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <string>

class HttpClient : public llhttp_t
{
public:
  HttpClient(const std::filesystem::path& path);

  ~HttpClient();

public:
  void send(std::string_view version,
            std::string_view method,
            std::string_view path,
            std::string_view body);

  MOCK_METHOD(void,
              on_recv,
              (uint16_t status, std::string_view body),
              ());

public:
  static int handle_on_message_begin(llhttp_t* http);

  static int handle_on_body(llhttp_t* http, const char* at, size_t length);

  static int handle_on_message_complete(llhttp_t* http);

private:
  void do_recv(boost::system::error_code ec, std::size_t size);

private:
  static llhttp_settings_t settings_;
  boost::asio::io_service ios_;
  boost::asio::io_service::work work_;
  boost::asio::local::stream_protocol::socket sock_;
  std::string content_;
  std::array<char, 1024> buffer_;
  std::thread th_;
};
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

class Client
{
public:
  class MessageHandler
  {
  public:
    virtual ~MessageHandler() = default;

  public:
    virtual void on_recv(std::string_view data) = 0;
  };

public:
  Client(MessageHandler* handler)
    : handler_(handler)
  {
  }

  virtual ~Client() = default;

public:
  virtual void send(std::string_view data) = 0;

protected:
  MessageHandler* handler_;
};

class HttpClient
  : public llhttp_t
  , public Client::MessageHandler
{
public:
  HttpClient(const std::filesystem::path& path);

  HttpClient(std::string_view host, uint16_t port);

  ~HttpClient();

public:
  void send(std::string_view version,
            std::string_view method,
            std::string_view path,
            std::string_view body);

  MOCK_METHOD(void, on_recv, (uint16_t status, std::string_view body), ());

public:
  static int handle_on_message_begin(llhttp_t* http);

  static int handle_on_body(llhttp_t* http, const char* at, size_t length);

  static int handle_on_message_complete(llhttp_t* http);

public:
  void on_recv(std::string_view data) override;

private:
  static llhttp_settings_t settings_;
  std::unique_ptr<Client> client_;
  std::string content_;
};

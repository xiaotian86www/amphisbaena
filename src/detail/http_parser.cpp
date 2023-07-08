#include "detail/http_parser.hpp"
#include "parser.hpp"

#include <iostream>
#include <llhttp.h>
#include <memory>
#include <mutex>

namespace translator {

static llhttp_settings_t g_settings;

static int
handle_on_method(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpParser*>(http);
  parser->request.method = std::string(at, length);
  return 0;
}

static int
handle_on_url(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpParser*>(http);
  parser->request.url = std::string(at, length);
  return 0;
}

static int
handle_on_body(llhttp_t* http, const char* at, size_t length)
{
  return 0;
}

static void
init()
{
  llhttp_settings_init(&g_settings);
  g_settings.on_method = handle_on_method;
  g_settings.on_body = handle_on_body;
  g_settings.on_url = handle_on_url;
}

static std::once_flag init_once_flag;

HttpParser::HttpParser()
{
  std::call_once(init_once_flag, init);
  llhttp_init(this, HTTP_REQUEST, &g_settings);
}

void
HttpParser::on_data(ScheduleRef sch,
                      CoroutineRef co,
                      std::shared_ptr<Connection> conn,
                      std::string_view data)
{
  enum llhttp_errno err = llhttp_execute(this, data.data(), data.length());
  if (err == HPE_OK) {
    std::cout << "url: " << request.url << ", method: " << request.method
              << std::endl;
  }
}

std::unique_ptr<Parser>
HttpParserFactory::create()
{
  return std::unique_ptr<HttpParser>();
}
}
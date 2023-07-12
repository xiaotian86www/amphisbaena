#include "detail/http_parser.hpp"
#include "context.hpp"
#include "detail/json_object.hpp"
#include "environment.hpp"
#include "object.hpp"
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
  parser->request->set_value("method", std::string_view(at, length));
  return HPE_OK;
}

static int
handle_on_url(llhttp_t* http, const char* at, size_t length)
{
  auto* parser = static_cast<HttpParser*>(http);
  parser->request->set_value("url", std::string_view(at, length));
  return HPE_OK;
}

static int
handle_on_body(llhttp_t* http, const char* at, size_t length)
{
  return HPE_OK;
}

static int
handle_on_message_complete(llhttp_t* http)
{
  auto parser = static_cast<HttpParser*>(http);
  Environment env;
  env.sch = parser->sch;
  env.co = parser->co;
  env.session = parser->session;

  std::string response_name;
  response_name += parser->request->get_value("method", "");
  response_name += " ";
  response_name += parser->request->get_value("url", "");

  env.object_pool.add(parser->request->get_value("url", ""),
                      std::move(parser->request));

  parser->request = std::make_unique<JsonObject>();

  env.object_pool.get(response_name, env);

  return HPE_OK;
}

static void
init()
{
  llhttp_settings_init(&g_settings);
  g_settings.on_method = handle_on_method;
  g_settings.on_body = handle_on_body;
  g_settings.on_url = handle_on_url;
  g_settings.on_message_complete = handle_on_message_complete;
}
void
HttpSession::reply(ScheduleRef sch, CoroutineRef co, const Object& data)
{
}

HttpParser::HttpParser()
{
  static std::once_flag init_once_flag;
  std::call_once(init_once_flag, init);
  llhttp_init(this, HTTP_REQUEST, &g_settings);

  request = std::make_unique<JsonObject>();
}

void
HttpParser::on_data(ScheduleRef sch,
                    CoroutineRef co,
                    ConnectionRef conn,
                    std::string_view data)
{
  enum llhttp_errno err = llhttp_execute(this, data.data(), data.length());
  // if (err == HPE_OK) {
  //   std::cout << "url: " << request->get_value("url", "") << ", "
  //             << "method: " << request->get_value("method", "") << std::endl;
  // }
}

std::shared_ptr<Parser>
HttpParserFactory::create()
{
  return std::make_shared<HttpParser>();
}
}
#include "http_builder.hpp"
#include "common/http_parser/http_parser.hpp"
#include "environment.hpp"
#include "log.hpp"

namespace amphisbaena {
HttpBuilder::HttpBuilder(std::shared_ptr<ServerFactory> server_factory)
  : http_parser_(std::make_unique<HttpServer>(server_factory, *this))
{
}

void
HttpBuilder::on_recv(ScheduleRef sch,
                     CoroutineRef co,
                     SessionPtr session,
                     MessagePtr message)
{
  auto request_head = message->get_head();
  auto request_body = message->get_body();

  LOG_DEBUG("method: {}, url: {}, body: {}",
            request_head->get_string("method"),
            request_head->get_string("url"),
            request_body->to_string());

  std::string pattern;
  pattern += request_head->get_string("method");
  pattern += " ";
  pattern += request_head->get_string("url");

  MessagePtr response;

  Environment env{ sch, co, session->up, session };

  try {
    response = MessageBuilder::create(env, pattern, message);
    session->up = env.up;

    auto response_head = response->get_head();

    response_head->set_value("code", HTTP_STATUS_OK);
    response_head->set_value("version", request_head->get_string("version"));

  } catch (...) {
    response = handle_error(request_head->get_string("version"));
  }

  LOG_DEBUG("code: {}, body: {}",
            response->get_head()->get_int("code"),
            response->get_body()->to_string());

  session->send(response);
}

MessagePtr
HttpBuilder::handle_error(std::string_view version)
{
  auto response = amphisbaena::MessageFactory::create("http");
  auto response_head = response->get_head();
  auto response_body = response->get_body();

  try {
    throw;
  } catch (const NotFoundException& ec) {
    response_head->set_value("code", HTTP_STATUS_NOT_FOUND);
    response_body->set_value("description", ec.what());
  } catch (const NoKeyException& ec) {
    response_head->set_value("code", HTTP_STATUS_BAD_REQUEST);
    response_body->set_value("description", ec.what());
  } catch (const TypeExecption& ec) {
    response_head->set_value("code", HTTP_STATUS_BAD_REQUEST);
    response_body->set_value("description", ec.what());
  } catch (const UnknownKeyException& ec) {
    response_head->set_value("code", HTTP_STATUS_BAD_REQUEST);
    response_body->set_value("description", ec.what());
  } catch (const Exception& ec) {
    response_head->set_value("code", HTTP_STATUS_INTERNAL_SERVER_ERROR);
    response_body->set_value("description", ec.what());
  }

  response_head->set_value("version", version);

  return response;
}

}
#include "builder.hpp"
#include "object.hpp"
#include "parser.hpp"
#include "schedule.hpp"

#include <llhttp.h>
#include <memory>
#include <string_view>

namespace translator {

class HttpSession
  : public std::enable_shared_from_this<HttpSession>
  , public Session
{
public:
  HttpSession(ConnectionRef conn);

public:
  void reply(ScheduleRef sch, CoroutineRef co, const Object& data) override;

private:
  ConnectionRef conn_;
};

class HttpParser
  : public Parser
  , public llhttp_t
{
public:
  HttpParser(ScheduleRef sch, CoroutineRef co, ConnectionRef conn);
  ~HttpParser() override = default;

public:
  void on_data(std::string_view data) override;

  void handle();

  void set_field(std::string_view name, std::string_view value);

private:
  void handle_error(llhttp_status_t status);

  void handle_success(const Object& object);

private:
  SessionPtr session_;
  ObjectPtr request_;
};

class HttpParserFactory : public ParserFactory
{
public:
  std::shared_ptr<Parser> create(ScheduleRef sch, CoroutineRef co, ConnectionRef conn) override
  {
    return std::make_shared<HttpParser>(sch, co, conn);
  }
};
}
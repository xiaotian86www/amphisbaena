#include "object.hpp"
#include "parser.hpp"
#include "processor.hpp"
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
  HttpParser();
  ~HttpParser() override = default;

public:
  void on_data(ScheduleRef sch,
               CoroutineRef co,
               ConnectionRef conn,
               std::string_view data) override;

public:
  ObjectPtr request;
  ScheduleRef sch;
  CoroutineRef co;
  SessionPtr session;
};

class HttpParserFactory : public ParserFactory
{
public:
  std::shared_ptr<Parser> create() override;
};
}
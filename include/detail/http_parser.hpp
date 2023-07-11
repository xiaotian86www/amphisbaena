#include "processor.hpp"
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
  HttpSession(std::shared_ptr<Connection> conn);

public:
  void reply(ScheduleRef sch,
             CoroutineRef co,
             const ResponseData& data) override;

private:
  std::shared_ptr<Connection> conn_;
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
               std::shared_ptr<Connection> conn,
               std::string_view data) override;

public:
  RequestData request;
  ScheduleRef sch;
  CoroutineRef co;
  std::shared_ptr<HttpSession> session;
};

class HttpParserFactory : public ParserFactory
{
public:
  std::shared_ptr<Parser> create() override;
};
}
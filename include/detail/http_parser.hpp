#include "processor.hpp"
#include "parser.hpp"

#include <llhttp.h>
#include <memory>
#include <string_view>

namespace translator {

class HttpSession
  : public std::enable_shared_from_this<HttpSession>
  , public Session
{
public:
  void reply(ScheduleRef sch,
             CoroutineRef co,
             const ResponseData& data) override;
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

private:
  std::shared_ptr<HttpSession> session_;
};

class HttpParserFactory : public ParserFactory
{
public:
  std::unique_ptr<Parser> create() override;
};
}
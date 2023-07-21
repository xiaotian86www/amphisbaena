#pragma once

#include <map>
#include <memory>
#include <string_view>

#include "schedule.hpp"
#include "server.hpp"

namespace translator {
class Parser
{
public:
  Parser(ScheduleRef sch, CoroutineRef co, ConnectionRef conn)
    : sch_(sch)
    , co_(co)
    , conn_(conn)
  {
  }

  virtual ~Parser() = default;

public:
  virtual void on_data(std::string_view data) = 0;

protected:
  ScheduleRef sch_;
  CoroutineRef co_;
  ConnectionRef conn_;
};

class ParserFactory
{
public:
  virtual ~ParserFactory() = default;

public:
  virtual std::shared_ptr<Parser> create(ScheduleRef sch,
                                         CoroutineRef co,
                                         ConnectionRef conn) = 0;
};
} // namespace translator

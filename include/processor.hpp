#pragma once

#include <memory>
#include <rapidjson/document.h>

#include "schedule.hpp"

namespace translator {

struct RequestData
{
  std::string method;
  std::string url;
  rapidjson::Document data;
};

struct ResponseData
{};

class Session
{
public:
  virtual ~Session() = default;

public:
  virtual void reply(ScheduleRef sch,
                     CoroutineRef co,
                     const ResponseData& data) = 0;
};

typedef std::shared_ptr<Session> SessionPtr;

class SessionRef
{
public:
  SessionRef() {}
  
  SessionRef(std::shared_ptr<Session> session)
    : session_(session)
  {
  }

  SessionRef(std::weak_ptr<Session> session)
    : session_(session)
  {
  }

public:
  void reply(ScheduleRef sch, CoroutineRef co, const ResponseData& data)
  {
    if (auto session = session_.lock()) {
      session->reply(sch, co, data);
    }
  }

private:
  std::weak_ptr<Session> session_;
};

class Processor
{
public:
  virtual ~Processor() = default;

public:
  virtual void handle(ScheduleRef sch,
                      CoroutineRef co,
                      SessionRef session,
                      const RequestData& data) = 0;
};

class ProcessorFactory : public std::enable_shared_from_this<ProcessorFactory>
{
public:
  using ctor_prototype = std::shared_ptr<Processor>();
  using ctor_function = std::function<ctor_prototype>;

public:
  virtual ~ProcessorFactory() = default;

public:
  std::shared_ptr<Processor> create(std::string_view key);

  void registe(std::string_view key, ctor_function ctor);

  void unregiste(std::string_view key);

private:
  std::unordered_map<std::string_view, ctor_function> ctors_;
};
}
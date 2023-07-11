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

class Processor
{
public:
  virtual ~Processor() = default;

public:
  virtual void handle(ScheduleRef sch,
                      CoroutineRef co,
                      std::shared_ptr<Session> session,
                      const RequestData& data) = 0;
};

class ProcessorFactory
  : public std::enable_shared_from_this<ProcessorFactory>
{
public:
  using ctor_prototype = std::shared_ptr<Processor>();
  using ctor_function = std::function<ctor_prototype>;

public:
  virtual ~ProcessorFactory() = default;

public:
  std::shared_ptr<Processor> create(std::string_view key);

  void registe(std::string_view key,
               ctor_function ctor);

  void unregiste(std::string_view key);

private:
  std::unordered_map<std::string_view, ctor_function>
    ctors_;
};
}
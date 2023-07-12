#pragma once

#include <memory>

#include "schedule.hpp"
#include "object.hpp"

namespace translator {

class Session
{
public:
  virtual ~Session() = default;

public:
  virtual void reply(ScheduleRef sch,
                     CoroutineRef co,
                     const Object& data) = 0;
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
  void reply(ScheduleRef sch, CoroutineRef co, const Object& data)
  {
    if (auto session = session_.lock()) {
      session->reply(sch, co, data);
    }
  }

private:
  std::weak_ptr<Session> session_;
};

class Environment;

class ObjectBuilder
{
public:
  using ctor_prototype = ObjectPtr(Environment&);
  using ctor_function = std::function<ctor_prototype>;

public:
  void registe(std::string_view name, ctor_function&& func);

  ObjectPtr create(std::string_view name, Environment& env) const;

private:
  std::unordered_map<std::string_view, ctor_function> ctors_;
};

typedef std::shared_ptr<ObjectBuilder> ObjectBuilderPtr;
}
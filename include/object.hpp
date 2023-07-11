#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>
#include <unordered_map>

#include <boost/lexical_cast.hpp>

namespace translator {
class Group;

class Object
{
public:
  virtual ~Object() = default;

public:
  virtual int32_t get_value(std::string_view name,
                            int32_t default_value) const = 0;

  virtual std::string_view get_value(std::string_view name,
                                     std::string_view default_value) const = 0;

  virtual void set_value(std::string_view name, int32_t value) = 0;

  virtual void set_value(std::string_view name, std::string_view value) = 0;

  virtual Group* get_group(std::string_view name) = 0;

  virtual const Group* get_group(std::string_view name) const = 0;
};

typedef std::unique_ptr<Object> ObjectPtr;

class Group
{
public:
  virtual ~Group() = default;

public:
  virtual Object* at(std::size_t index) = 0;

  virtual const Object* at(std::size_t index) const = 0;

  virtual std::size_t get_size() const = 0;

  template<typename IterType_>
  IterType_ begin();

  template<typename IterType_>
  IterType_ end();

  template<typename ConstIterType_>
  ConstIterType_ begin() const;

  template<typename ConstIterType_>
  ConstIterType_ end() const;
};

class Environment;

class ObjectFactory
{
public:
  typedef std::function<std::unique_ptr<Object>(Environment&)> ctor_func_type;

public:
  void registe(std::string_view name, ctor_func_type&& func);

  ObjectPtr create(std::string_view name, Environment& env) const;

private:
  std::unordered_map<std::string_view, ctor_func_type> ctors_;
};

typedef std::shared_ptr<ObjectFactory> ObjectFactoryPtr;

class ObjectPool
{
public:
  void add(std::string_view name, ObjectPtr&& object);

  Object* get(std::string_view name, Environment& env) const;

private:
  mutable std::unordered_map<std::string_view, ObjectPtr> objects_;
};

} // namespace translator

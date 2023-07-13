#pragma once

#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace translator {
class Group;
typedef std::unique_ptr<Group> GroupPtr;

class Node
{
public:
  virtual ~Node() = default;

public:
  virtual int32_t get_value(std::string_view name,
                            int32_t default_value) const = 0;

  virtual std::string_view get_value(std::string_view name,
                                     std::string_view default_value) const = 0;

  virtual void set_value(std::string_view name, int32_t value) = 0;

  virtual void set_value(std::string_view name, std::string_view value) = 0;

  virtual GroupPtr get_group(std::string_view name) = 0;

  virtual const GroupPtr get_group(std::string_view name) const = 0;
};

typedef std::unique_ptr<Node> NodePtr;
typedef std::unique_ptr<const Node> ConstNodePtr;

class Group
{
public:
  virtual ~Group() = default;

public:
  virtual NodePtr at(std::size_t index) = 0;

  virtual const NodePtr at(std::size_t index) const = 0;

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

class Object
{
public:
  virtual ~Object() = default;

public:
  virtual int32_t get_value(std::string_view name,
                            int32_t default_value) const noexcept = 0;

  virtual std::string_view get_value(std::string_view name,
                                     std::string_view default_value) const noexcept = 0;

  virtual int32_t get_int(std::string_view name) const = 0;

  virtual std::string_view get_string(std::string_view name) const = 0;

  virtual void set_value(std::string_view name, int32_t value) = 0;

  virtual void set_value(std::string_view name, std::string_view value) = 0;

  virtual NodePtr get_node(std::string_view name) = 0;

  virtual ConstNodePtr get_node(std::string_view name) const = 0;

  virtual std::string to_string() const = 0;

  virtual void from_string(std::string_view str) = 0;

  virtual std::string to_binary() const = 0;

  virtual void from_binary(std::string_view bin) = 0;

  virtual void clear() = 0;

public:
  std::string name;
};

typedef std::unique_ptr<Object> ObjectPtr;

class Environment;

class ObjectPool
{
public:
  void add(std::string_view name, ObjectPtr&& object);

  const Object& get(std::string_view name, Environment& env) const;

private:
  mutable std::unordered_map<std::string_view, ObjectPtr> objects_;
};

class NoKeyException : public std::exception
{
public:
  NoKeyException(std::string_view name)
    : name_(name)
  {
    what_ += name;
    what_ += " required";
  }

public:
  const char* what() const noexcept override { return what_.c_str(); }

  std::string_view name() const noexcept { return name_; }

private:
  std::string name_;
  std::string what_;
};

class NotFoundException : public std::exception
{
public:
  NotFoundException(std::string_view name)
    : name_(name)
  {
    what_ += name;
    what_ += " not found";
  }

public:
  const char* what() const noexcept override { return what_.c_str(); }

  std::string_view name() const noexcept { return name_; }

private:
  std::string name_;
  std::string what_;
};

} // namespace translator

#pragma once

#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace translator {

class Object;
typedef std::unique_ptr<Object> ObjectPtr;
typedef std::unique_ptr<const Object> ConstObjectPtr;

class Group;
typedef std::unique_ptr<Group> GroupPtr;

class Object
{
public:
  virtual ~Object() = default;

public:
  virtual int32_t get_value(std::string_view name,
                            int32_t default_value) const = 0;

  virtual std::string_view get_value(std::string_view name,
                                     std::string_view default_value) const = 0;

  virtual double get_value(std::string_view name, double default_value) const = 0;

  virtual int32_t get_int(std::string_view name) const = 0;

  virtual std::string_view get_string(std::string_view name) const = 0;

  virtual double get_double(std::string_view name) const = 0;

  virtual void set_value(std::string_view name, int32_t value) = 0;

  virtual void set_value(std::string_view name, std::string_view value) = 0;

  virtual void set_value(std::string_view name, double value) = 0;

  virtual ObjectPtr get_object(std::string_view name) = 0;

  virtual ConstObjectPtr get_object(std::string_view name) const = 0;

  virtual ObjectPtr get_or_set_object(std::string_view name) = 0;

  virtual GroupPtr get_group(std::string_view name) = 0;

  virtual const GroupPtr get_group(std::string_view name) const = 0;
};

class Group
{
public:
  virtual ~Group() = default;

public:
  virtual ObjectPtr at(std::size_t index) = 0;

  virtual const ObjectPtr at(std::size_t index) const = 0;

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

class Message
{
public:
  virtual ~Message() = default;

public:
  virtual Object& get_head() = 0;

  virtual const Object& get_head() const = 0;

  virtual Object& get_body() = 0;

  virtual const Object& get_body() const = 0;

  virtual Object& get_tail() = 0;

  virtual const Object& get_tail() const = 0;

  virtual std::string to_string() const = 0;

  virtual void from_string(std::string_view str) = 0;

  virtual std::string to_binary() const = 0;

  virtual void from_binary(std::string_view bin) = 0;

  virtual void clear() = 0;

public:
  std::string name;
};

typedef std::shared_ptr<Message> MessagePtr;

class Environment;

// class MessagePool
// {
// public:
//   void add(std::string_view name, MessagePtr message);

//   MessagePtr get(std::string_view name, Environment& env) const;

// private:
//   mutable std::unordered_map<std::string_view, MessagePtr> messages_;
// };

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

class TypeExecption : public std::exception
{
public:
  TypeExecption(std::string_view name, std::string_view type)
    : name_(name)
    , type_(type)
  {
    what_ += name;
    what_ += " required ";
    what_ += type;
  }

public:
  const char* what() const noexcept override { return what_.c_str(); }

  std::string_view name() const noexcept { return name_; }

private:
  std::string name_;
  std::string type_;
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

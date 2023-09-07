/**
 * @file exception.hpp
 * @author duchang (xiaotian86www@163.com)
 * @brief 异常类
 * @version 0.1
 * @date 2023-08-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */
 
#include <exception>
#include <string>
#include <string_view>

namespace amphisbaena {
class Exception : public std::exception
{};

class NoKeyException : public Exception
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

class TypeExecption : public Exception
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

class NotFoundException : public Exception
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

class UnknownKeyException : public Exception
{
public:
  UnknownKeyException(std::string_view name)
    : name_(name)
  {
    what_ += "unknown field: ";
    what_ += name;
  }

public:
  const char* what() const noexcept override { return what_.c_str(); }

  std::string_view name() const noexcept { return name_; }

private:
  std::string name_;
  std::string what_;
};

class CouldnotLoadException : public Exception
{
public:
  CouldnotLoadException(std::string_view name, std::string_view desc)
    : name_(name)
  {
    what_ += "could not load ";
    what_ += name_;
    what_ += ": ";
    what_ += desc;
  }

public:
  const char* what() const noexcept override { return what_.c_str(); }

private:
  std::string name_;
  std::string what_;
};

class TimeoutException : public Exception
{
public:
  const char* what() const noexcept override { return "timeout"; }
};

class PluginNotFoundException : public Exception
{
public:
  PluginNotFoundException(std::string_view name)
    : name_(name)
  {
    what_ += name;
    what_ += " plugin not found";
  }

public:
  const char* what() const noexcept override { return what_.c_str(); }

  std::string_view name() const noexcept { return name_; }

private:
  std::string name_;
  std::string what_;
};

class PluginExistedException : public Exception
{
public:
  PluginExistedException(std::string_view name)
    : name_(name)
  {
    what_ += name;
    what_ += " plugin has existed";
  }

public:
  const char* what() const noexcept override { return what_.c_str(); }

  std::string_view name() const noexcept { return name_; }

private:
  std::string name_;
  std::string what_;
};
}
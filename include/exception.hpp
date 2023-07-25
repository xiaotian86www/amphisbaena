#include <exception>
#include <string>
#include <string_view>

namespace translator {
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

}
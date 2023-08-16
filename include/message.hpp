#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "exception.hpp"

namespace amphisbaena {

// TODO 调整FieldType组织方式，简化field_type_name、check_field_type调用
enum class FieldType
{
  kUnknown,
  kInt,
  kString,
  kDouble
};

constexpr std::string_view
field_type_name(FieldType type)
{
  switch (type) {
    case FieldType::kUnknown:
      return "unknown";
    case FieldType::kInt:
      return "integer";
    case FieldType::kString:
      return "string";
    case FieldType::kDouble:
      return "double";
  }
}

template<typename T>
constexpr bool
check_field_type(FieldType type)
{
  return false;
}

template<>
constexpr bool
check_field_type<int32_t>(FieldType type)
{
  return type == FieldType::kInt;
}

template<>
constexpr bool
check_field_type<double>(FieldType type)
{
  return type == FieldType::kDouble;
}

template<>
constexpr bool
check_field_type<std::string_view>(FieldType type)
{
  return type == FieldType::kString;
}

class Object;
typedef std::unique_ptr<Object> ObjectPtr;
typedef std::unique_ptr<const Object> ConstObjectPtr;

class Group;
typedef std::unique_ptr<Group> GroupPtr;

class Message;
typedef std::shared_ptr<Message> MessagePtr;

class Object
{
public:
  class ConstIterator
  {
  public:
    virtual ~ConstIterator() = default;

  public:
    virtual std::string_view get_name() = 0;

    virtual FieldType get_type() = 0;

    virtual int32_t get_int() = 0;

    virtual std::string_view get_string() = 0;

    virtual double get_double() = 0;

    virtual bool operator!=(const ConstIterator& right) = 0;

    virtual ConstIterator& operator++() = 0;
  };

  class ConstIteratorWrap
  {
  public:
    ConstIteratorWrap(std::unique_ptr<ConstIterator> it)
      : it_(std::move(it))
    {
    }

  public:
    std::string_view get_name() { return it_->get_name(); }

    FieldType get_type() { return it_->get_type(); }

    int32_t get_int() { return it_->get_int(); }

    std::string_view get_string() { return it_->get_string(); }

    double get_double() { return it_->get_double(); }

    bool operator!=(const ConstIteratorWrap& right)
    {
      return (*it_) != (*right.it_);
    }

    ConstIteratorWrap& operator++()
    {
      ++(*it_);
      return *this;
    }

  private:
    std::unique_ptr<ConstIterator> it_;
  };

public:
  virtual void copy_from(ObjectPtr right);

  virtual ~Object() = default;

public:
  virtual int32_t get_value(std::string_view name,
                            int32_t default_value) const = 0;

  virtual std::string_view get_value(std::string_view name,
                                     std::string_view default_value) const = 0;

  virtual double get_value(std::string_view name,
                           double default_value) const = 0;

  virtual int32_t get_int(std::string_view name) const = 0;

  virtual std::string_view get_string(std::string_view name) const = 0;

  virtual double get_double(std::string_view name) const = 0;

  virtual void set_value(std::string_view name, int32_t value) = 0;

  virtual void set_value(std::string_view name, std::string_view value) = 0;

  virtual void set_value(std::string_view name, double value) = 0;

  virtual std::size_t count() const = 0;

  virtual ConstIteratorWrap begin() const = 0;

  virtual ConstIteratorWrap end() const = 0;

  virtual ObjectPtr get_object(std::string_view name) = 0;

  virtual ConstObjectPtr get_object(std::string_view name) const = 0;

  virtual ObjectPtr get_or_set_object(std::string_view name) = 0;

  virtual GroupPtr get_group(std::string_view name) = 0;

  virtual const GroupPtr get_group(std::string_view name) const = 0;

public:
  virtual void from_string(std::string_view str) = 0;
  
  virtual std::string to_string() const = 0;

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
  virtual MessagePtr clone() const = 0;

  virtual ~Message() = default;

public:
  virtual ObjectPtr get_head() = 0;

  virtual ConstObjectPtr get_head() const = 0;

  virtual ObjectPtr get_body() = 0;

  virtual ConstObjectPtr get_body() const = 0;

  virtual ObjectPtr get_tail() = 0;

  virtual ConstObjectPtr get_tail() const = 0;

  // virtual std::string to_string() const = 0;

  // virtual void from_string(std::string_view str) = 0;

  // virtual std::string to_binary() const = 0;

  // virtual void from_binary(std::string_view bin) = 0;

  virtual void clear() = 0;

  // public:
  //   std::string name;
};

enum class MessageType
{
  // kUnknown = 0,
  kJson = 1,
  kFix = 2
};

class MessageFactory
{
public:
  virtual ~MessageFactory() = default;

public:
  virtual MessagePtr create() = 0;

  virtual std::string_view name() = 0;

public:
  /**
   * @brief 注册
   * 
   * @param factory 
   *
   * 要求线程安全
   */
  static void registe(std::shared_ptr<MessageFactory> factory);

  /**
   * @brief 取消全部注册
   * 
   * 要求线程安全
   */
  static void unregiste();

  /**
   * @brief 取消注册
   * 
   * @param factory 
   *
   * 要求线程安全
   */
  static void unregiste(std::shared_ptr<MessageFactory> factory);

  static MessagePtr create(std::string_view type);

private:
  static std::shared_ptr<
    std::map<std::string, std::shared_ptr<MessageFactory>, std::less<>>>
    ctors_;
};
} // namespace amphisbaena

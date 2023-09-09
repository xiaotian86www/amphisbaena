/**
 * @file message.hpp
 * @author duchang (xiaotian86www@163.com)
 * @brief 消息类
 * @version 0.1
 * @date 2023-08-23
 *
 * @copyright Copyright (c) 2023
 *
 */

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

/**
 * @brief 字段类型
 *
 */
enum class FieldType
{
  kUnknown, ///< 未知类型
  kInt,     ///< 整数类型
  kString,  ///< 字符串类型
  kDouble   ///< 浮点数类型
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

  return "";
}

template<typename T>
constexpr bool
check_field_type(FieldType /* type */)
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

/**
 * @brief 单条记录接口
 *
 */
class Object
{
public:
  /**
   * @brief 常量字段迭代器接口
   *
   */
  class ConstIterator
  {
  public:
    virtual ~ConstIterator() = default;

  public:
    /**
     * @brief 字段名
     *
     * @return std::string_view
     */
    virtual std::string_view get_name() = 0;

    /**
     * @brief 字段类型
     *
     * @return @link FieldType @endlink
     */
    virtual FieldType get_type() = 0;

    /**
     * @brief 整数类型值
     *
     * @return int32_t
     */
    virtual int32_t get_int() = 0;

    /**
     * @brief 字符串类型值
     *
     * @return std::string_view
     */
    virtual std::string_view get_string() = 0;

    /**
     * @brief 浮点类型值
     *
     * @return double
     */
    virtual double get_double() = 0;

    /**
     * @brief 比较字段常量迭代器是否相等
     *
     * @param right
     * @return true
     * @return false
     */
    virtual bool operator!=(const ConstIterator& right) = 0;

    /**
     * @brief 迭代器自增
     *
     * @return ConstIterator&
     */
    virtual ConstIterator& operator++() = 0;
  };

  /**
   * @brief 常量迭代器包装类
   *
   */
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
  /**
   * @brief 拷贝
   *
   * @param right
   */
  virtual void copy_from(ObjectPtr right);

  virtual ~Object() = default;

public:
  /**
   * @brief 获取整数值
   *
   * 字段名不存在或者字段类型不匹配时返回默认值
   *
   * @param name 字段名
   * @param default_value 默认值
   * @return int32_t
   */
  virtual int32_t get_value(std::string_view name,
                            int32_t default_value) const = 0;

  /**
   * @brief 获取字符串值
   *
   * 字段名不存在或者字段类型不匹配时返回默认值
   *
   * @param name 字段名
   * @param default_value 默认值
   * @return std::string_view
   */
  virtual std::string_view get_value(std::string_view name,
                                     std::string_view default_value) const = 0;

  /**
   * @brief 获取浮点数值
   *
   * 字段名不存在或者字段类型不匹配时返回默认值
   *
   * @param name 字段名
   * @param default_value 默认值
   * @return double
   */
  virtual double get_value(std::string_view name,
                           double default_value) const = 0;

  /**
   * @brief 获取整数值
   *
   * @param name 字段名
   * @return int32_t
   *
   * @exception NoKeyException 字段名不存在
   * @exception TypeExecption 字段类型不匹配
   */
  virtual int32_t get_int(std::string_view name) const = 0;

  /**
   * @brief 获取字符串值
   *
   * @param name 字段名
   * @return std::string_view
   *
   * @exception NoKeyException 字段名不存在
   * @exception TypeExecption 字段类型不匹配
   */
  virtual std::string_view get_string(std::string_view name) const = 0;

  /**
   * @brief 获取浮点数值
   *
   * @param name 字段名
   * @return double
   *
   * @exception NoKeyException 字段名不存在
   * @exception TypeExecption 字段类型不匹配
   */
  virtual double get_double(std::string_view name) const = 0;

  virtual Object* set_value(std::string_view name, int32_t value) = 0;

  virtual Object* set_value(std::string_view name,
                              std::string_view value) = 0;

  virtual Object* set_value(std::string_view name, double value) = 0;

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

  virtual void clear() = 0;
};

/**
 * @brief 消息工厂
 *
 */
class MessageFactory
{
public:
  virtual ~MessageFactory() = default;

public:
  /**
   * @brief 创建消息
   *
   * @return MessagePtr
   */
  virtual MessagePtr create() = 0;

  /**
   * @brief 消息类型
   *
   * @return std::string_view
   */
  virtual std::string_view name() = 0;

public:
  /**
   * @brief 注册
   *
   * @param factory
   *
   * @note 要求线程安全
   */
  static void registe(std::shared_ptr<MessageFactory> factory);

  /**
   * @brief 取消全部注册
   *
   * @note 要求线程安全
   */
  static void unregiste();

  /**
   * @brief 取消注册
   *
   * @param factory
   *
   * @note 要求线程安全
   */
  static void unregiste(std::shared_ptr<MessageFactory> factory);

  /**
   * @brief 创建消息
   *
   * @param type 消息类型
   * @return MessagePtr
   */
  static MessagePtr create(std::string_view type);

private:
  static std::shared_ptr<
    std::map<std::string, std::shared_ptr<MessageFactory>, std::less<>>>
    ctors_;
};
} // namespace amphisbaena

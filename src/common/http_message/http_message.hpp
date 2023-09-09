/**
 * @file http_message.hpp
 * @author duchang (xiaotian86www@163.com)
 * @brief http消息类
 * @version 0.1
 * @date 2023-08-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <memory>
#include <rapidjson/allocators.h>
#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <rapidjson/rapidjson.h>

#include "message.hpp"

namespace amphisbaena {

typedef rapidjson::Document RapidDocument;
typedef rapidjson::Value RapidValue;
typedef rapidjson::MemoryPoolAllocator<> RapidAllocator;

/**
 * @brief 单条Json记录
 * 
 */
class JsonObject : public Object
{
public:
  class ConstIterator : public Object::ConstIterator
  {
  public:
    ConstIterator() = default;

    ConstIterator(RapidValue::ConstMemberIterator it);

  public:
    std::string_view get_name() override;

    FieldType get_type() override;

    int32_t get_int() override;

    std::string_view get_string() override;

    double get_double() override;

    bool operator!=(const Object::ConstIterator& right) override;

    Object::ConstIterator& operator++() override;

  private:
    RapidValue::ConstMemberIterator it_;
  };

public:
  JsonObject(RapidValue& value);

public:
  int32_t get_value(std::string_view name,
                    int32_t default_value) const override;

  std::string_view get_value(std::string_view name,
                             std::string_view default_value) const override;

  double get_value(std::string_view name, double default_value) const override;

  int32_t get_int(std::string_view name) const override;

  std::string_view get_string(std::string_view name) const override;

  double get_double(std::string_view name) const override;

  Object* set_value(std::string_view name, int32_t value) override;

  Object* set_value(std::string_view name, std::string_view value) override;

  Object* set_value(std::string_view name, double value) override;

  std::size_t count() const override;

  ConstIteratorWrap begin() const override;

  ConstIteratorWrap end() const override;

  ObjectPtr get_object(std::string_view name) override;

  ConstObjectPtr get_object(std::string_view name) const override;

  ObjectPtr get_or_set_object(std::string_view name) override;

  GroupPtr get_group(std::string_view name) override;

  const GroupPtr get_group(std::string_view name) const override;

public:
  void from_string(std::string_view str) override;

  std::string to_string() const override;

private:
  template<typename Type_>
  Type_ get_value(std::string_view name, Type_ default_value) const;

  template<typename Type_>
  Type_ get_value(std::string_view name) const;

  template<typename Type_>
  Object* set_value(std::string_view name, Type_ value);

private:
  RapidValue& value_;
};

/**
 * @brief http消息
 * 
 */
class HttpMessage : public Message
{
public:
  HttpMessage();

  HttpMessage(const HttpMessage& right);

  MessagePtr clone() const override;

public:
  ObjectPtr get_head() override;

  ConstObjectPtr get_head() const override;

  ObjectPtr get_body() override;

  ConstObjectPtr get_body() const override;

  ObjectPtr get_tail() override;

  ConstObjectPtr get_tail() const override;

  // std::string to_string() const override { return {}; }

  // std::string to_binary() const override { return {}; }

  // void from_string(std::string_view str) override {}

  // void from_binary(std::string_view bin) override {}

  void clear() override;

private:
  RapidDocument doc_;
};

class HttpMessageFactory : public MessageFactory
{
public:
  MessagePtr create() override;

  std::string_view name() override;
};
}
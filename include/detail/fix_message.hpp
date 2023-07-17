#pragma once

#include <quickfix/FieldTypes.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"
#include <quickfix/DataDictionary.h>
#include <quickfix/Field.h>
#include <quickfix/FieldMap.h>
#include <quickfix/FixFields.h>
#include <quickfix/Message.h>
#pragma GCC diagnostic pop

#include "message.hpp"

namespace translator {

template<typename Value_>
constexpr bool
check_type(FIX::TYPE::Type type)
{
  return false;
}

template<>
constexpr bool
check_type<std::string_view>(FIX::TYPE::Type type)
{
  return type == FIX::TYPE::Type::String;
}

template<>
constexpr bool
check_type<int32_t>(FIX::TYPE::Type type)
{
  return type == FIX::TYPE::Type::Int;
}

constexpr std::string_view
type_name(FIX::TYPE::Type type)
{
  switch (type) {
    case FIX::TYPE::Type::String:
      return "String";
    case FIX::TYPE::Type::Int:
      return "Int";
    default:
      return "Unknown";
  }
}

class FixObject : public Object
{
public:
  FixObject(const FIX::DataDictionary& dd, FIX::FieldMap& fields)
    : dd_(dd)
    , fields_(fields)
  {
  }

public:
  int32_t get_value(std::string_view name, int32_t default_value) const override
  {
    return get_value<FIX::IntField>(name, default_value);
  }

  std::string_view get_value(std::string_view name,
                             std::string_view default_value) const override
  {
    return get_value<FIX::StringField>(name, default_value);
  }

  int32_t get_int(std::string_view name) const override
  {
    return get_value<FIX::IntField, int32_t>(name);
  }

  std::string_view get_string(std::string_view name) const override
  {
    return get_value<FIX::StringField, std::string_view>(name);
  }

  void set_value(std::string_view name, int32_t value) override
  {
    set_value<FIX::IntField>(name, value);
  }

  void set_value(std::string_view name, std::string_view value) override
  {
    set_value<FIX::StringField, const std::string&>(name, std::string(value));
  }

  ObjectPtr get_object(std::string_view name) override { return {}; }

  ConstObjectPtr get_object(std::string_view name) const override { return {}; }

  ObjectPtr get_or_set_object(std::string_view name) override { return {}; }

  GroupPtr get_group(std::string_view name) override { return {}; }

  const GroupPtr get_group(std::string_view name) const override { return {}; }

private:
  template<typename Field_, typename Value_>
  Value_ get_value(std::string_view name, Value_ default_value) const
  {
    auto name_ = std::string(name);
    int tag;
    if (!dd_.getFieldTag(name_, tag))
      return default_value;

    if (!fields_.isSetField(tag))
      return default_value;

    FIX::TYPE::Type type;
    if (!dd_.getFieldType(tag, type))
      return default_value;

    if (!check_type<Value_>(type))
      return default_value;

    // 不能使用getFieldIfSet方法，因为需要传入临时变量field，无法返回string_view类型数据

    return static_cast<const Field_&>(fields_.getFieldRef(tag)).getValue();
  }

  template<typename Field_, typename Value_>
  Value_ get_value(std::string_view name) const
  {
    auto name_ = std::string(name);
    int tag;
    if (!dd_.getFieldTag(name_, tag))
      throw NoKeyException(name);

    if (!fields_.isSetField(tag))
      throw NoKeyException(name);

    FIX::TYPE::Type type;
    if (!dd_.getFieldType(tag, type))
      throw NoKeyException(name);

    if (!check_type<Value_>(type))
      throw TypeExecption(name, type_name(type));

    return static_cast<const Field_&>(fields_.getFieldRef(tag)).getValue();
  }

  template<typename Field_, typename Value_>
  void set_value(std::string_view name, Value_ value)
  {
    auto name_ = std::string(name);
    int tag;
    if (!dd_.getFieldTag(name_, tag))
      return;

    Field_ field(tag, value);
    return fields_.setField(field);
  }

private:
  const FIX::DataDictionary& dd_;
  FIX::FieldMap& fields_;
};

class FixMessage : public Message
{
public:
  FixMessage(const FIX::DataDictionary& dd)
    : dd_(dd)
    , head_(dd_, message_.getHeader())
    , body_(dd_, message_)
    , tail_(dd_, message_.getTrailer())
  {
  }

public:
  Object& get_head() override { return head_; }

  const Object& get_head() const override { return head_; }

  Object& get_body() override { return body_; }

  const Object& get_body() const override { return body_; }

  Object& get_tail() override { return tail_; }

  const Object& get_tail() const override { return tail_; }

  std::string to_string() const override { return {}; }

  void from_string(std::string_view str) override {}

  std::string to_binary() const override { return {}; }

  void from_binary(std::string_view bin) override {}

  void clear() override {}

public:
  FIX::Message& message() { return message_; }

  const FIX::Message& message() const { return message_; }

private:
  const FIX::DataDictionary& dd_;
  FIX::Message message_;
  FixObject head_;
  FixObject body_;
  FixObject tail_;
};
}
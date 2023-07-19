#pragma once

#include <map>
#include <string_view>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdynamic-exception-spec"
#include <quickfix/DataDictionary.h>
#include <quickfix/Field.h>
#include <quickfix/FieldMap.h>
#include <quickfix/FieldTypes.h>
#include <quickfix/FixFields.h>
#include <quickfix/Message.h>
#pragma GCC diagnostic pop

#include "message.hpp"

namespace translator {

namespace detail {
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
  return type == FIX::TYPE::Type::String || type == FIX::TYPE::Type::Char ||
         type == FIX::TYPE::Type::UtcTimeStamp;
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
    case FIX::TYPE::Type::Char:
    case FIX::TYPE::Type::UtcTimeStamp:
      return "String";
    case FIX::TYPE::Type::Int:
      return "Int";
    default:
      return "Unknown";
  }
}

class get_field_info
{
public:
  std::tuple<int, FIX::TYPE::Type> operator()(std::string_view name)
  {
    if (auto iter = tags_.find(name); iter != tags_.end()) {
      return iter->second;
    } else {
      return { 0, FIX::TYPE::Type::Unknown };
    }
  }

public:
  static void init(std::string_view url);

  static void init(std::istream& is);

private:
  static std::map<std::string, std::tuple<int, FIX::TYPE::Type>, std::less<>>
    tags_;
};
}

class FixObject : public Object
{
public:
  FixObject(FIX::FieldMap& fields)
    : fields_(fields)
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
    auto [tag, type] = detail::get_field_info()(name_);
    if (!tag)
      return default_value;

    if (!fields_.isSetField(tag))
      return default_value;

    if (!detail::check_type<Value_>(type))
      return default_value;

    // 不能使用getFieldIfSet方法，因为需要传入临时变量field，无法返回string_view类型数据

    return static_cast<const Field_&>(fields_.getFieldRef(tag)).getValue();
  }

  template<typename Field_, typename Value_>
  Value_ get_value(std::string_view name) const
  {
    auto name_ = std::string(name);
    auto [tag, type] = detail::get_field_info()(name_);
    if (!tag)
      throw NoKeyException(name);

    if (!fields_.isSetField(tag))
      throw NoKeyException(name);

    if (!detail::check_type<Value_>(type))
      throw TypeExecption(name, detail::type_name(type));

    return static_cast<const Field_&>(fields_.getFieldRef(tag)).getValue();
  }

  template<typename Field_, typename Value_>
  void set_value(std::string_view name, Value_ value)
  {
    auto name_ = std::string(name);
    auto [tag, type] = detail::get_field_info()(name_);
    if (!tag)
      return;

    Field_ field(tag, value);
    return fields_.setField(field);
  }

private:
  FIX::FieldMap& fields_;
};

class FixMessage : public Message
{
public:
  FixMessage()
    : head_(message_.getHeader())
    , body_(message_)
    , tail_(message_.getTrailer())
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
  FIX::Message message_;
  FixObject head_;
  FixObject body_;
  FixObject tail_;
};
}
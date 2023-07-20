#pragma once

#include "message.hpp"

#include <memory>
#include <rapidjson/allocators.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

namespace translator {
namespace detail {
template<typename Type_>
constexpr bool
check_type(const rapidjson::Value& value);

template<>
constexpr bool
check_type<int32_t>(const rapidjson::Value& value)
{
  return value.IsInt();
}

template<>
constexpr bool
check_type<std::string_view>(const rapidjson::Value& value)
{
  return value.IsString();
}

template<>
constexpr bool
check_type<double>(const rapidjson::Value& value)
{
  return value.IsDouble();
}

constexpr std::string_view
type_name(const rapidjson::Value& value)
{
  switch (value.GetType()) {
    case rapidjson::Type::kStringType:
      return "String";
    case rapidjson::Type::kNumberType:
      return "Number";
    default:
      return "Unknown";
  }
}

template<typename Type_>
constexpr Type_
get_value(const rapidjson::Value& value)
{
  return value.Get<Type_>();
}

template<>
constexpr std::string_view
get_value<std::string_view>(const rapidjson::Value& value)
{
  return { value.GetString(), value.GetStringLength() };
}

template<typename Type_>
constexpr void
set_value(rapidjson::Document::AllocatorType& allocator,
          rapidjson::Value& value,
          Type_ v)
{
  value.Set<Type_>(v);
}

template<>
constexpr void
set_value<std::string_view>(rapidjson::Document::AllocatorType& allocator,
                            rapidjson::Value& value,
                            std::string_view v)
{
  value.SetString(v.data(), v.size(), allocator);
}

template<typename Type_>
inline rapidjson::Value
create_value(rapidjson::Document::AllocatorType& allocator, Type_ type)
{
  return rapidjson::Value(type);
}

template<>
inline rapidjson::Value
create_value<std::string_view>(rapidjson::Document::AllocatorType& allocator,
                               std::string_view value)
{
  return rapidjson::Value(value.data(), value.size(), allocator);
}
}

class JsonObject : public Object
{
public:
  JsonObject(rapidjson::Document::AllocatorType& allocator,
             rapidjson::Value& value)
    : allocator_(allocator)
    , value_(value)
  {
  }

public:
  int32_t get_value(std::string_view name, int32_t default_value) const override
  {
    return get_value<>(name, default_value);
  }

  std::string_view get_value(std::string_view name,
                             std::string_view default_value) const override
  {
    return get_value<>(name, default_value);
  }

  double get_value(std::string_view name, double default_value) const override
  {
    return get_value<>(name, default_value);
  }

  int32_t get_int(std::string_view name) const override
  {
    return get_value<int32_t>(name);
  }

  std::string_view get_string(std::string_view name) const override
  {
    return get_value<std::string_view>(name);
  }

  double get_double(std::string_view name) const override
  {
    return get_value<double>(name);
  }

  void set_value(std::string_view name, int32_t value) override
  {
    set_value<>(name, value);
  }

  void set_value(std::string_view name, std::string_view value) override
  {
    set_value<>(name, value);
  }

  void set_value(std::string_view name, double value) override
  {
    set_value<>(name, value);
  }

  ObjectPtr get_object(std::string_view name) override
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd()) {
      if (iter->value.IsObject())
        return ObjectPtr(new JsonObject(allocator_, iter->value));
      else
        throw TypeExecption(name, "Object");
    } else {
      throw NoKeyException(name);
    }
  }

  ConstObjectPtr get_object(std::string_view name) const override
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd()) {
      if (iter->value.IsObject())
        return ConstObjectPtr(new JsonObject(allocator_, iter->value));
      else
        throw TypeExecption(name, "Object");
    } else {
      throw NoKeyException(name);
    }
  }

  ObjectPtr get_or_set_object(std::string_view name) override
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd()) {
      if (!iter->value.IsObject()) {
        iter->value = rapidjson::Value(rapidjson::Type::kObjectType);
      }
      return std::make_unique<JsonObject>(allocator_, iter->value);
    } else {
      auto& value =
        value_.AddMember(rapidjson::StringRef(name.data(), name.size()),
                         rapidjson::Value(rapidjson::Type::kObjectType),
                         allocator_);

      return std::make_unique<JsonObject>(allocator_, value);
    }
  }

  GroupPtr get_group(std::string_view name) override { return GroupPtr(); }

  const GroupPtr get_group(std::string_view name) const override
  {
    return GroupPtr();
  }

private:
  template<typename Type_>
  Type_ get_value(std::string_view name, Type_ default_value) const
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd() && detail::check_type<Type_>(iter->value)) {
      return detail::get_value<Type_>(iter->value);
    } else {
      return default_value;
    }
  }

  template<typename Type_>
  Type_ get_value(std::string_view name) const
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd()) {
      if (detail::check_type<Type_>(iter->value))
        return detail::get_value<Type_>(iter->value);
      else
        throw TypeExecption(name, detail::type_name(iter->value));
    } else {
      throw NoKeyException(name);
    }
  }

  template<typename Type_>
  void set_value(std::string_view name, Type_ value)
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd()) {
      detail::set_value(allocator_, iter->value, value);
    } else {
      value_.AddMember(rapidjson::StringRef(name.data(), name.size()),
                       detail::create_value(allocator_, value),
                       allocator_);
    }
  }

private:
  rapidjson::Document::AllocatorType& allocator_;
  rapidjson::Value& value_;
};

class JsonMessage : public Message
{
public:
  JsonMessage()
    : doc_(rapidjson::Type::kObjectType)
    , head_(doc_.GetAllocator(),
            doc_.AddMember("head",
                           rapidjson::Value(rapidjson::Type::kObjectType),
                           doc_.GetAllocator()))
    , body_(doc_.GetAllocator(),
            doc_.AddMember("body",
                           rapidjson::Value(rapidjson::Type::kObjectType),
                           doc_.GetAllocator()))
    , tail_(doc_.GetAllocator(),
            doc_.AddMember("tail",
                           rapidjson::Value(rapidjson::Type::kObjectType),
                           doc_.GetAllocator()))
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

  std::string to_binary() const override { return {}; }

  void from_string(std::string_view str) override {}

  void from_binary(std::string_view bin) override {}

  void clear() override { doc_.RemoveAllMembers(); }

private:
  rapidjson::Document doc_;
  JsonObject head_;
  JsonObject body_;
  JsonObject tail_;
};
}
#pragma once

#include "message.hpp"

#include <memory>
#include <rapidjson/allocators.h>
#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <rapidjson/rapidjson.h>

namespace translator {

typedef rapidjson::Document RapidDocument;
typedef rapidjson::Value RapidValue;

namespace detail {
template<typename Type_>
constexpr bool
check_type(const RapidValue& value);

template<>
constexpr bool
check_type<int32_t>(const RapidValue& value)
{
  return value.IsInt();
}

template<>
constexpr bool
check_type<std::string_view>(const RapidValue& value)
{
  return value.IsString();
}

template<>
constexpr bool
check_type<double>(const RapidValue& value)
{
  return value.IsDouble();
}

constexpr std::string_view
type_name(const RapidValue& value)
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
get_value(const RapidValue& value)
{
  return value.Get<Type_>();
}

template<>
constexpr std::string_view
get_value<std::string_view>(const RapidValue& value)
{
  return { value.GetString(), value.GetStringLength() };
}

template<typename Type_>
constexpr void
set_value(RapidDocument::AllocatorType& allocator, RapidValue& value, Type_ v)
{
  value.Set<Type_>(v);
}

template<>
constexpr void
set_value<std::string_view>(RapidDocument::AllocatorType& allocator,
                            RapidValue& value,
                            std::string_view v)
{
  value.SetString(v.data(), v.size(), allocator);
}

template<typename Type_>
inline RapidValue
create_value(RapidDocument::AllocatorType& allocator, Type_ type)
{
  return RapidValue(type);
}

template<>
inline RapidValue
create_value<std::string_view>(RapidDocument::AllocatorType& allocator,
                               std::string_view value)
{
  return RapidValue(value.data(), value.size(), allocator);
}
}

class JsonObject : public Object
{
public:
  JsonObject(RapidDocument::AllocatorType& allocator, RapidValue& value)
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
        iter->value = RapidValue(rapidjson::Type::kObjectType);
      }
      return std::make_unique<JsonObject>(allocator_, iter->value);
    } else {
      auto& value =
        value_.AddMember(rapidjson::StringRef(name.data(), name.size()),
                         RapidValue(rapidjson::Type::kObjectType),
                         allocator_);

      return std::make_unique<JsonObject>(allocator_, value);
    }
  }

  GroupPtr get_group(std::string_view name) override { return GroupPtr(); }

  const GroupPtr get_group(std::string_view name) const override
  {
    return GroupPtr();
  }

public:
  void from_string(std::string_view str);

  std::string to_string() const;

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
      value_.AddMember(RapidValue(name.data(), name.size(), allocator_),
                       detail::create_value(allocator_, value),
                       allocator_);
    }
  }

private:
  RapidDocument::AllocatorType& allocator_;
  RapidValue& value_;
};

class JsonMessage : public Message
{
public:
  JsonMessage()
    : doc_(rapidjson::Type::kObjectType)
  {
    doc_.AddMember(
      "head", RapidValue(rapidjson::Type::kObjectType), doc_.GetAllocator());
    doc_.AddMember(
      "body", RapidValue(rapidjson::Type::kObjectType), doc_.GetAllocator());
    doc_.AddMember(
      "tail", RapidValue(rapidjson::Type::kObjectType), doc_.GetAllocator());
  }

public:
  ObjectPtr get_head() override
  {
    return std::make_unique<JsonObject>(doc_.GetAllocator(),
                                        doc_.FindMember("head")->value);
  }

  ConstObjectPtr get_head() const override
  {
    return const_cast<JsonMessage*>(this)->get_head();
  }

  ObjectPtr get_body() override
  {
    return std::make_unique<JsonObject>(doc_.GetAllocator(),
                                        doc_.FindMember("body")->value);
  }

  ConstObjectPtr get_body() const override
  {
    return const_cast<JsonMessage*>(this)->get_body();
  }

  ObjectPtr get_tail() override
  {
    return std::make_unique<JsonObject>(doc_.GetAllocator(),
                                        doc_.FindMember("tail")->value);
  }

  ConstObjectPtr get_tail() const override
  {
    return const_cast<JsonMessage*>(this)->get_tail();
  }

  // std::string to_string() const override { return {}; }

  // std::string to_binary() const override { return {}; }

  // void from_string(std::string_view str) override {}

  // void from_binary(std::string_view bin) override {}

  void clear() override { doc_.RemoveAllMembers(); }

private:
  RapidDocument doc_;
};
}
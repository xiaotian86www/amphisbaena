
#include <cassert>
#include <cstdint>
#include <fmt/core.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <stdexcept>
#include <string_view>

#include "http_message.hpp"

namespace amphisbaena {

namespace detail {
template<typename Type_>
inline Type_
get_value(const RapidValue& value)
{
  return value.Get<Type_>();
}

template<>
inline std::string_view
get_value<std::string_view>(const RapidValue& value)
{
  return { value.GetString(), value.GetStringLength() };
}

template<typename Type_>
inline void
set_value(RapidDocument::AllocatorType& /* allocator */,
          RapidValue& value,
          Type_ v)
{
  value.Set<Type_>(v);
}

template<>
inline void
set_value<std::string_view>(RapidDocument::AllocatorType& allocator,
                            RapidValue& value,
                            std::string_view v)
{
  value.SetString(v.data(), v.size(), allocator);
}

template<typename Type_>
inline RapidValue
create_value(RapidDocument::AllocatorType& /* allocator */, Type_ type)
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

inline FieldType
get_type(const RapidValue& value)
{
  switch (value.GetType()) {
    case rapidjson::Type::kNumberType:
      if (value.IsInt())
        return FieldType::kInt;
      else if (value.IsDouble())
        return FieldType::kDouble;
      else
        return FieldType::kUnknown;
    case rapidjson::Type::kStringType:
      return FieldType::kString;
    default:
      return FieldType::kUnknown;
  }
}
}

static RapidAllocator g_allocator;

JsonObject::ConstIterator::ConstIterator(RapidValue::ConstMemberIterator it)
  : it_(it)
{
}

std::string_view
JsonObject::ConstIterator::get_name()
{
  return detail::get_value<std::string_view>(it_->name);
}

FieldType
JsonObject::ConstIterator::get_type()
{
  return detail::get_type(it_->value);
}

int32_t
JsonObject::ConstIterator::get_int()
{
  return detail::get_value<int32_t>(it_->value);
}

std::string_view
JsonObject::ConstIterator::get_string()
{
  return detail::get_value<std::string_view>(it_->value);
}

double
JsonObject::ConstIterator::get_double()
{
  return detail::get_value<double>(it_->value);
}

bool
JsonObject::ConstIterator::operator!=(const Object::ConstIterator& right)
{
  assert(dynamic_cast<const ConstIterator*>(&right));
  const auto& right_it = static_cast<const ConstIterator&>(right);
  return it_ != right_it.it_;
}

Object::ConstIterator&
JsonObject::ConstIterator::operator++()
{
  ++it_;
  return *this;
}

JsonObject::JsonObject(RapidValue& value)
  : value_(value)
{
}

int32_t
JsonObject::get_value(std::string_view name, int32_t default_value) const
{
  return get_value<>(name, default_value);
}

std::string_view
JsonObject::get_value(std::string_view name,
                      std::string_view default_value) const
{
  return get_value<>(name, default_value);
}

double
JsonObject::get_value(std::string_view name, double default_value) const
{
  return get_value<>(name, default_value);
}

int32_t
JsonObject::get_int(std::string_view name) const
{
  return get_value<int32_t>(name);
}

std::string_view
JsonObject::get_string(std::string_view name) const
{
  return get_value<std::string_view>(name);
}

double
JsonObject::get_double(std::string_view name) const
{
  return get_value<double>(name);
}

void
JsonObject::set_value(std::string_view name, int32_t value)
{
  set_value<>(name, value);
}

void
JsonObject::set_value(std::string_view name, std::string_view value)
{
  set_value<>(name, value);
}

void
JsonObject::set_value(std::string_view name, double value)
{
  set_value<>(name, value);
}

std::size_t
JsonObject::count() const
{
  return value_.MemberCount();
}

Object::ConstIteratorWrap
JsonObject::begin() const
{
  return Object::ConstIteratorWrap(
    std::make_unique<ConstIterator>(value_.MemberBegin()));
}

Object::ConstIteratorWrap
JsonObject::end() const
{
  return Object::ConstIteratorWrap(
    std::make_unique<ConstIterator>(value_.MemberEnd()));
}

ObjectPtr
JsonObject::get_object(std::string_view name)
{
  if (auto iter =
        value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
      iter != value_.MemberEnd()) {
    if (iter->value.IsObject())
      return ObjectPtr(new JsonObject(iter->value));
    else
      throw TypeExecption(name, "Object");
  } else {
    throw NoKeyException(name);
  }
}

ConstObjectPtr
JsonObject::get_object(std::string_view name) const
{
  if (auto iter =
        value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
      iter != value_.MemberEnd()) {
    if (iter->value.IsObject())
      return ConstObjectPtr(new JsonObject(iter->value));
    else
      throw TypeExecption(name, "Object");
  } else {
    throw NoKeyException(name);
  }
}

ObjectPtr
JsonObject::get_or_set_object(std::string_view name)
{
  if (auto iter =
        value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
      iter != value_.MemberEnd()) {
    if (!iter->value.IsObject()) {
      iter->value = RapidValue(rapidjson::Type::kObjectType);
    }
    return std::make_unique<JsonObject>(iter->value);
  } else {
    value_.AddMember(rapidjson::StringRef(name.data(), name.size()),
                     RapidValue(rapidjson::Type::kObjectType),
                     g_allocator);

    iter = value_.FindMember(rapidjson::StringRef(name.data(), name.size()));

    return std::make_unique<JsonObject>(iter->value);
  }
}

GroupPtr
JsonObject::get_group(std::string_view /* name */)
{
  return GroupPtr();
}

const GroupPtr
JsonObject::get_group(std::string_view /* name */) const
{
  return GroupPtr();
}

void
JsonObject::from_string(std::string_view str)
{
  RapidDocument doc(rapidjson::Type::kObjectType, &g_allocator);
  rapidjson::ParseResult ok = doc.Parse(str.data(), str.size());
  if (!ok) {
    std::string what = fmt::format("parse error: {} ({})",
                                   rapidjson::GetParseError_En(ok.Code()),
                                   ok.Offset());

    throw std::invalid_argument(what);
  }

  value_.Swap(doc);
}

std::string
JsonObject::to_string() const
{
  rapidjson::StringBuffer sb;
  rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

  value_.Accept(writer);
  return std::string(sb.GetString(), sb.GetLength());
}

template<typename Type_>
Type_
JsonObject::get_value(std::string_view name, Type_ default_value) const
{
  if (auto iter =
        value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
      iter != value_.MemberEnd() &&
      check_field_type<Type_>(detail::get_type(iter->value))) {
    return detail::get_value<Type_>(iter->value);
  } else {
    return default_value;
  }
}

template<typename Type_>
Type_
JsonObject::get_value(std::string_view name) const
{
  if (auto iter =
        value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
      iter != value_.MemberEnd()) {
    if (check_field_type<Type_>(detail::get_type(iter->value)))
      return detail::get_value<Type_>(iter->value);
    else
      throw TypeExecption(name, field_type_name(detail::get_type(iter->value)));
  } else {
    throw NoKeyException(name);
  }
}

template<typename Type_>
void
JsonObject::set_value(std::string_view name, Type_ value)
{
  if (auto iter =
        value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
      iter != value_.MemberEnd()) {
    detail::set_value(g_allocator, iter->value, value);
  } else {
    value_.AddMember(RapidValue(name.data(), name.size(), g_allocator),
                     detail::create_value(g_allocator, value),
                     g_allocator);
  }
}

HttpMessage::HttpMessage()
  : doc_(rapidjson::Type::kObjectType, &g_allocator)
{
  doc_.AddMember("head", RapidValue(rapidjson::Type::kObjectType), g_allocator);
  doc_.AddMember("body", RapidValue(rapidjson::Type::kObjectType), g_allocator);
  doc_.AddMember("tail", RapidValue(rapidjson::Type::kObjectType), g_allocator);
}

HttpMessage::HttpMessage(const HttpMessage& right)
  : HttpMessage()
{
  *get_head() = *right.get_head();
  *get_body() = *right.get_body();
  *get_tail() = *right.get_tail();
}

MessagePtr
HttpMessage::clone() const
{
  return std::make_shared<HttpMessage>(*this);
}

ObjectPtr
HttpMessage::get_head()
{
  return std::make_unique<JsonObject>(doc_.FindMember("head")->value);
}

ConstObjectPtr
HttpMessage::get_head() const
{
  return const_cast<HttpMessage*>(this)->get_head();
}

ObjectPtr
HttpMessage::get_body()
{
  return std::make_unique<JsonObject>(doc_.FindMember("body")->value);
}

ConstObjectPtr
HttpMessage::get_body() const
{
  return const_cast<HttpMessage*>(this)->get_body();
}

ObjectPtr
HttpMessage::get_tail()
{
  return std::make_unique<JsonObject>(doc_.FindMember("tail")->value);
}

ConstObjectPtr
HttpMessage::get_tail() const
{
  return const_cast<HttpMessage*>(this)->get_tail();
}

// std::string to_string() const { return {}; }

// std::string to_binary() const { return {}; }

// void from_string(std::string_view str) {}

// void from_binary(std::string_view bin) {}

void
HttpMessage::clear()
{
  doc_.RemoveAllMembers();
}

MessagePtr
HttpMessageFactory::create()
{
  return std::make_shared<HttpMessage>();
}

std::string_view
HttpMessageFactory::name()
{
  return "Http";
}

}

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <filesystem>
#include <fstream>
#include <ios>
#include <memory>
#include <string>
#include <string_view>

#include "fix_message.hpp"
#include "fix_field.hpp"

namespace amphisbaena {
namespace detail {

std::tuple<int, FIX::TYPE::Type>
get_field_info(std::string_view name)
{
  if (auto iter = g_fields_by_name.find(name); iter != g_fields_by_name.end()) {
    return iter->second;
  } else {
    return { 0, FIX::TYPE::Type::Unknown };
  }
}

std::tuple<std::string_view, FIX::TYPE::Type>
get_field_info(int tag)
{
  if (auto iter = g_fields_by_tag.find(tag); iter != g_fields_by_tag.end()) {
    return { std::get<0>(iter->second), std::get<1>(iter->second) };
  } else {
    return { std::string_view(), FIX::TYPE::Type::Unknown };
  }
}

constexpr FieldType
get_type(FIX::TYPE::Type type)
{
  switch (type) {
    case FIX::TYPE::Type::String:
    case FIX::TYPE::Type::Char:
    case FIX::TYPE::Type::UtcTimeStamp:
      return FieldType::kString;
    case FIX::TYPE::Type::Int:
      return FieldType::kInt;
    case FIX::TYPE::Type::Qty:
    case FIX::TYPE::Type::Price:
      return FieldType::kDouble;
    default:
      return FieldType::kUnknown;
  }
}

FIX::TYPE::Type
get_type(std::string_view type)
{
  if (type == "STRING")
    return FIX::TYPE::String;
  if (type == "CHAR")
    return FIX::TYPE::Char;
  if (type == "PRICE")
    return FIX::TYPE::Price;
  if (type == "INT")
    return FIX::TYPE::Int;
  if (type == "AMT")
    return FIX::TYPE::Amt;
  if (type == "QTY")
    return FIX::TYPE::Qty;
  if (type == "CURRENCY")
    return FIX::TYPE::Currency;
  if (type == "MULTIPLEVALUESTRING")
    return FIX::TYPE::MultipleValueString;
  if (type == "MULTIPLESTRINGVALUE")
    return FIX::TYPE::MultipleStringValue;
  if (type == "MULTIPLECHARVALUE")
    return FIX::TYPE::MultipleCharValue;
  if (type == "EXCHANGE")
    return FIX::TYPE::Exchange;
  if (type == "UTCTIMESTAMP")
    return FIX::TYPE::UtcTimeStamp;
  if (type == "BOOLEAN")
    return FIX::TYPE::Boolean;
  if (type == "LOCALMKTDATE")
    return FIX::TYPE::LocalMktDate;
  if (type == "DATA")
    return FIX::TYPE::Data;
  if (type == "FLOAT")
    return FIX::TYPE::Float;
  if (type == "PRICEOFFSET")
    return FIX::TYPE::PriceOffset;
  if (type == "MONTHYEAR")
    return FIX::TYPE::MonthYear;
  if (type == "DAYOFMONTH")
    return FIX::TYPE::DayOfMonth;
  if (type == "UTCDATE")
    return FIX::TYPE::UtcDate;
  if (type == "UTCDATEONLY")
    return FIX::TYPE::UtcDateOnly;
  if (type == "UTCTIMEONLY")
    return FIX::TYPE::UtcTimeOnly;
  if (type == "NUMINGROUP")
    return FIX::TYPE::NumInGroup;
  if (type == "PERCENTAGE")
    return FIX::TYPE::Percentage;
  if (type == "SEQNUM")
    return FIX::TYPE::SeqNum;
  if (type == "LENGTH")
    return FIX::TYPE::Length;
  if (type == "COUNTRY")
    return FIX::TYPE::Country;
  if (type == "TIME")
    return FIX::TYPE::UtcTimeStamp;
  return FIX::TYPE::Unknown;
}

}

FixObject::ConstIterator::ConstIterator(FIX::FieldMap::const_iterator it)
  : it_(it)
{
}

std::string_view
FixObject::ConstIterator::get_name()
{
  auto [name, type] = detail::get_field_info(it_->getTag());
  return name;
}

FieldType
FixObject::ConstIterator::get_type()
{
  auto [name, type] = detail::get_field_info(it_->getTag());
  return detail::get_type(type);
}

int32_t
FixObject::ConstIterator::get_int()
{
  return static_cast<const FIX::IntField&>(*it_).getValue();
}

std::string_view
FixObject::ConstIterator::get_string()
{
  return static_cast<const FIX::StringField&>(*it_).getValue();
}

double
FixObject::ConstIterator::get_double()
{
  return static_cast<const FIX::DoubleField&>(*it_).getValue();
}

bool
FixObject::ConstIterator::operator!=(const Object::ConstIterator& right)
{
  assert(dynamic_cast<const ConstIterator*>(&right));
  const auto& right_it = static_cast<const ConstIterator&>(right);
  return it_ != right_it.it_;
}

Object::ConstIterator&
FixObject::ConstIterator::operator++()
{
  ++it_;
  return *this;
}

FixObject::FixObject(FIX::FieldMap& fields)
  : fields_(fields)
{
}

int32_t
FixObject::get_value(std::string_view name, int32_t default_value) const
{
  return get_value<FIX::IntField>(name, default_value);
}

std::string_view
FixObject::get_value(std::string_view name,
                     std::string_view default_value) const
{
  return FixObject::get_value<FIX::StringField>(name, default_value);
}

double
FixObject::get_value(std::string_view name, double default_value) const
{
  return get_value<FIX::DoubleField>(name, default_value);
}

int32_t
FixObject::get_int(std::string_view name) const
{
  return get_value<FIX::IntField, int32_t>(name);
}

std::string_view
FixObject::get_string(std::string_view name) const
{
  return get_value<FIX::StringField, std::string_view>(name);
}

double
FixObject::get_double(std::string_view name) const
{
  return get_value<FIX::DoubleField, double>(name);
}

void
FixObject::set_value(std::string_view name, int32_t value)
{
  set_value<FIX::IntField>(name, value);
}

void
FixObject::set_value(std::string_view name, std::string_view value)
{
  set_value<FIX::StringField, const std::string&>(name, std::string(value));
}

void
FixObject::set_value(std::string_view name, double value)
{
  set_value<FIX::DoubleField, double>(name, value);
}

std::size_t
FixObject::count() const
{
  return fields_.totalFields();
}

Object::ConstIteratorWrap
FixObject::begin() const
{
  return Object::ConstIteratorWrap(
    std::make_unique<ConstIterator>(fields_.begin()));
}

Object::ConstIteratorWrap
FixObject::end() const
{
  return Object::ConstIteratorWrap(
    std::make_unique<ConstIterator>(fields_.end()));
}

ObjectPtr
FixObject::get_object(std::string_view /* name */)
{
  return {};
}

ConstObjectPtr
FixObject::get_object(std::string_view /* name */) const
{
  return {};
}

ObjectPtr
FixObject::get_or_set_object(std::string_view /* name */)
{
  return {};
}

GroupPtr
FixObject::get_group(std::string_view /* name */)
{
  return {};
}

const GroupPtr
FixObject::get_group(std::string_view /* name */) const
{
  return {};
}

void
FixObject::from_string(std::string_view /* str */)
{
}

std::string
FixObject::to_string() const
{
  return std::string();
}

template<typename Field_, typename Value_>
Value_
FixObject::get_value(std::string_view name, Value_ default_value) const
{
  auto name_ = std::string(name);
  auto [tag, type] = detail::get_field_info(name_);
  if (!tag)
    return default_value;

  if (!fields_.isSetField(tag))
    return default_value;

  if (!check_field_type<Value_>(detail::get_type(type)))
    return default_value;

  // 不能使用getFieldIfSet方法，因为需要传入临时变量field，无法返回string_view类型数据

  return static_cast<const Field_&>(fields_.getFieldRef(tag)).getValue();
}

template<typename Field_, typename Value_>
Value_
FixObject::get_value(std::string_view name) const
{
  auto name_ = std::string(name);
  auto [tag, type] = detail::get_field_info(name_);
  if (!tag)
    throw UnknownKeyException(name);

  if (!fields_.isSetField(tag))
    throw NoKeyException(name);

  if (!check_field_type<Value_>(detail::get_type(type)))
    throw TypeExecption(name, field_type_name(detail::get_type(type)));

  return static_cast<const Field_&>(fields_.getFieldRef(tag)).getValue();
}

template<typename Field_, typename Value_>
void
FixObject::set_value(std::string_view name, Value_ value)
{
  auto name_ = std::string(name);
  auto [tag, type] = detail::get_field_info(name_);
  if (!tag)
    return;

  Field_ field(tag, value);
  return fields_.setField(field);
}

FixMessage::FixMessage() {}

FixMessage::FixMessage(const FIX::Message& msg)
  : fix_message(msg)
{
}

FixMessage::FixMessage(const FixMessage& right)
  : fix_message(right.fix_message)
{
}

MessagePtr
FixMessage::clone() const
{
  return std::make_shared<FixMessage>(*this);
}

ObjectPtr
FixMessage::get_head()
{
  return std::make_unique<FixObject>(fix_message.getHeader());
}

ConstObjectPtr
FixMessage::get_head() const
{
  return std::make_unique<FixObject>(
    const_cast<FIX::Header&>(fix_message.getHeader()));
}

ObjectPtr
FixMessage::get_body()
{
  return std::make_unique<FixObject>(fix_message);
}

ConstObjectPtr
FixMessage::get_body() const
{
  return std::make_unique<FixObject>(const_cast<FIX::Message&>(fix_message));
}

ObjectPtr
FixMessage::get_tail()
{
  return std::make_unique<FixObject>(fix_message.getTrailer());
}

ConstObjectPtr
FixMessage::get_tail() const
{
  return std::make_unique<FixObject>(
    const_cast<FIX::Trailer&>(fix_message.getTrailer()));
}

// std::string to_string() const { return {}; }

// void from_string(std::string_view str) {}

// std::string to_binary() const { return {}; }

// void from_binary(std::string_view bin) {}

void
FixMessage::clear()
{
  fix_message.clear();
}

MessagePtr
FixMessageFactory::create()
{
  return std::make_shared<FixMessage>();
}

std::string_view
FixMessageFactory::name()
{
  return "fix";
}

}
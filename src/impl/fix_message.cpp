
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <filesystem>
#include <fstream>
#include <ios>

#include "fix_message.hpp"

namespace translator {
namespace detail {
std::map<std::string, std::tuple<int, FIX::TYPE::Type>, std::less<>>
  get_field_info::tags_;

static FIX::TYPE::Type
get_field_type(boost::optional<std::string> type)
{
  if (!type)
    return FIX::TYPE::Unknown;
  if (*type == "STRING")
    return FIX::TYPE::String;
  if (*type == "CHAR")
    return FIX::TYPE::Char;
  if (*type == "PRICE")
    return FIX::TYPE::Price;
  if (*type == "INT")
    return FIX::TYPE::Int;
  if (*type == "AMT")
    return FIX::TYPE::Amt;
  if (*type == "QTY")
    return FIX::TYPE::Qty;
  if (*type == "CURRENCY")
    return FIX::TYPE::Currency;
  if (*type == "MULTIPLEVALUESTRING")
    return FIX::TYPE::MultipleValueString;
  if (*type == "MULTIPLESTRINGVALUE")
    return FIX::TYPE::MultipleStringValue;
  if (*type == "MULTIPLECHARVALUE")
    return FIX::TYPE::MultipleCharValue;
  if (*type == "EXCHANGE")
    return FIX::TYPE::Exchange;
  if (*type == "UTCTIMESTAMP")
    return FIX::TYPE::UtcTimeStamp;
  if (*type == "BOOLEAN")
    return FIX::TYPE::Boolean;
  if (*type == "LOCALMKTDATE")
    return FIX::TYPE::LocalMktDate;
  if (*type == "DATA")
    return FIX::TYPE::Data;
  if (*type == "FLOAT")
    return FIX::TYPE::Float;
  if (*type == "PRICEOFFSET")
    return FIX::TYPE::PriceOffset;
  if (*type == "MONTHYEAR")
    return FIX::TYPE::MonthYear;
  if (*type == "DAYOFMONTH")
    return FIX::TYPE::DayOfMonth;
  if (*type == "UTCDATE")
    return FIX::TYPE::UtcDate;
  if (*type == "UTCDATEONLY")
    return FIX::TYPE::UtcDateOnly;
  if (*type == "UTCTIMEONLY")
    return FIX::TYPE::UtcTimeOnly;
  if (*type == "NUMINGROUP")
    return FIX::TYPE::NumInGroup;
  if (*type == "PERCENTAGE")
    return FIX::TYPE::Percentage;
  if (*type == "SEQNUM")
    return FIX::TYPE::SeqNum;
  if (*type == "LENGTH")
    return FIX::TYPE::Length;
  if (*type == "COUNTRY")
    return FIX::TYPE::Country;
  if (*type == "TIME")
    return FIX::TYPE::UtcTimeStamp;
  return FIX::TYPE::Unknown;
}

static std::map<std::string, std::tuple<int, FIX::TYPE::Type>, std::less<>>
init_tags(boost::property_tree::ptree& pt)
{
  std::map<std::string, std::tuple<int, FIX::TYPE::Type>, std::less<>> tags;
  auto fields = pt.get_child_optional("fix.fields");
  if (fields) {
    for (auto& child : *fields) {
      auto attrs = child.second.get_child("<xmlattr>");
      auto name = attrs.get_optional<std::string>("name");
      auto number = attrs.get_optional<int>("number");
      auto type = get_field_type(attrs.get_optional<std::string>("type"));
      if (name && number) {
        tags[*name] = { *number, type };
      }
    }
  }
  return std::move(tags);
}

void
get_field_info::init(std::string_view url)
{
  boost::property_tree::ptree pt;
  boost::property_tree::xml_parser::read_xml(std::string(url), pt);
  tags_ = init_tags(pt);
}

void
get_field_info::init(std::istream& is)
{
  boost::property_tree::ptree pt;
  boost::property_tree::xml_parser::read_xml(is, pt);
  tags_ = init_tags(pt);
}
}
}
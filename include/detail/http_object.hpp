#pragma once

#include "object.hpp"

#include "rapidjson/document.h"

namespace translator {
class HttpNode : public Node
{
public:
  HttpNode(rapidjson::Document& doc, rapidjson::Value& value)
    : doc_(doc)
    , value_(value)
  {
  }

  HttpNode(const rapidjson::Document& doc, const rapidjson::Value& value)
    : doc_(const_cast<rapidjson::Document&>(doc))
    , value_(const_cast<rapidjson::Value&>(value))
  {
  }

public:
  int32_t get_value(std::string_view name, int32_t default_value) const override
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd() && iter->value.IsInt()) {
      return iter->value.GetInt();
    } else {
      return default_value;
    }
  }

  std::string_view get_value(std::string_view name,
                             std::string_view default_value) const override
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd() && iter->value.IsString()) {
      return { iter->value.GetString(), iter->value.GetStringLength() };
    } else {
      return default_value;
    }
  }

  void set_value(std::string_view name, int32_t value) override
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd()) {
      iter->value.SetInt(value);
    } else {
      value_.AddMember(rapidjson::StringRef(name.data(), name.size()),
                       rapidjson::Value(value),
                       doc_.GetAllocator());
    }
  }

  void set_value(std::string_view name, std::string_view value) override
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd()) {
      iter->value.SetString(value.data(), value.size(), doc_.GetAllocator());
    } else {
      value_.AddMember(
        rapidjson::StringRef(name.data(), name.size()),
        rapidjson::Value(value.data(), value.size(), doc_.GetAllocator()),
        doc_.GetAllocator());
    }
  }

  GroupPtr get_group(std::string_view name) override { return GroupPtr(); }

  const GroupPtr get_group(std::string_view name) const override
  {
    return GroupPtr();
  }

private:
  rapidjson::Document& doc_;
  rapidjson::Value& value_;
};

class HttpObject : public Object
{
public:
  int32_t get_value(std::string_view name, int32_t default_value) const override
  {
    if (auto iter =
          doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != doc_.MemberEnd() && iter->value.IsInt()) {
      return iter->value.GetInt();
    } else {
      return default_value;
    }
  }

  std::string_view get_value(std::string_view name,
                             std::string_view default_value) const override
  {
    if (auto iter =
          doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != doc_.MemberEnd() && iter->value.IsString()) {
      return { iter->value.GetString(), iter->value.GetStringLength() };
    } else {
      return default_value;
    }
  }

  void set_value(std::string_view name, int32_t value) override
  {
    if (auto iter =
          doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != doc_.MemberEnd()) {
      iter->value.SetInt(value);
    } else {
      doc_.AddMember(rapidjson::StringRef(name.data(), name.size()),
                     rapidjson::Value(value),
                     doc_.GetAllocator());
    }
  }

  void set_value(std::string_view name, std::string_view value) override
  {
    if (auto iter =
          doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != doc_.MemberEnd()) {
      iter->value.SetString(value.data(), value.size());
    } else {
      doc_.AddMember(rapidjson::StringRef(name.data(), name.size()),
                     rapidjson::StringRef(value.data(), value.size()),
                     doc_.GetAllocator());
    }
  }

  NodePtr get_node(std::string_view name) override
  {
    auto value =
      doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
    assert(value != doc_.MemberEnd());
    return std::make_unique<HttpNode>(doc_, value->value);
  }

  ConstNodePtr get_node(std::string_view name) const override
  {
    auto value =
      doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
    assert(value != doc_.MemberEnd());
    return std::make_unique<const HttpNode>(doc_, value->value);
  }

  std::string to_string() const override { return {}; }

private:
  rapidjson::Document doc_;
};
}
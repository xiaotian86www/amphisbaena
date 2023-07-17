#pragma once

#include "object.hpp"

#include <memory>
#include <rapidjson/allocators.h>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

namespace translator {

class JsonNode : public Node
{
public:
  JsonNode(rapidjson::Document::AllocatorType& allocator,
           rapidjson::Value& value)
    : allocator_(allocator)
    , value_(value)
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

  int32_t get_int(std::string_view name) const override
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd()) {
      if (iter->value.IsInt())
        return iter->value.GetInt();
      else
        throw TypeExecption(name, "Int");
    } else {
      throw NoKeyException(name);
    }
  }

  std::string_view get_string(std::string_view name) const override
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd()) {
      if (iter->value.IsString())
        return { iter->value.GetString(), iter->value.GetStringLength() };
      else
        throw TypeExecption(name, "String");
    } else {
      throw NoKeyException(name);
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
                       allocator_);
    }
  }

  void set_value(std::string_view name, std::string_view value) override
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd()) {
      iter->value.SetString(value.data(), value.size(), allocator_);
    } else {
      value_.AddMember(rapidjson::StringRef(name.data(), name.size()),
                       rapidjson::Value(value.data(), value.size(), allocator_),
                       allocator_);
    }
  }

  NodePtr get_node(std::string_view name) override
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd()) {
      if (iter->value.IsObject())
        return std::make_unique<JsonNode>(allocator_, iter->value);
      else
        throw TypeExecption(name, "Node");
    } else {
      throw NoKeyException(name);
    }
  }

  ConstNodePtr get_node(std::string_view name) const override
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd()) {
      if (iter->value.IsObject())
        return std::make_unique<const JsonNode>(allocator_, iter->value);
      else
        throw TypeExecption(name, "Node");
    } else {
      throw NoKeyException(name);
    }
  }

  NodePtr get_or_set_node(std::string_view name) override
  {
    if (auto iter =
          value_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        iter != value_.MemberEnd()) {
      if (!iter->value.IsObject()) {
        iter->value = rapidjson::Value(rapidjson::Type::kObjectType);
      }
      return std::make_unique<JsonNode>(allocator_, iter->value);
    } else {
      auto& value =
        value_.AddMember(rapidjson::StringRef(name.data(), name.size()),
                         rapidjson::Value(rapidjson::Type::kObjectType),
                         allocator_);

      return std::make_unique<JsonNode>(allocator_, value);
    }
  }

  GroupPtr get_group(std::string_view name) override { return GroupPtr(); }

  const GroupPtr get_group(std::string_view name) const override
  {
    return GroupPtr();
  }

private:
  rapidjson::Document::AllocatorType& allocator_;
  rapidjson::Value& value_;
};

class JsonObject : public Object
{
public:
  JsonObject()
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
  Node& get_head() override { return head_; }

  const Node& get_head() const override { return head_; }

  Node& get_body() override { return body_; }

  const Node& get_body() const override { return body_; }

  Node& get_tail() override { return tail_; }

  const Node& get_tail() const override { return tail_; }

  std::string to_string() const override { return {}; }

  std::string to_binary() const override { return {}; }

  void from_string(std::string_view str) override {}

  void from_binary(std::string_view bin) override {}

  void clear() override { doc_.RemoveAllMembers(); }

private:
  rapidjson::Document doc_;
  JsonNode head_;
  JsonNode body_;
  JsonNode tail_;
};
}
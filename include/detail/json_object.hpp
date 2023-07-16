#pragma once

#include "object.hpp"

#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

namespace translator {
class JsonNode : public Node
{
public:
  JsonNode(rapidjson::Document& doc, rapidjson::Value& value)
    : doc_(doc)
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

class JsonObject : public Object
{
public:
  JsonObject()
    : doc_(rapidjson::Type::kObjectType)
    , root_(doc_, doc_)
  {
  }

public:
  // int32_t get_value(std::string_view name,
  //                   int32_t default_value) const noexcept override
  // {
  //   if (auto iter =
  //         doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
  //       iter != doc_.MemberEnd() && iter->value.IsInt()) {
  //     return iter->value.GetInt();
  //   } else {
  //     return default_value;
  //   }
  // }

  // std::string_view get_value(
  //   std::string_view name,
  //   std::string_view default_value) const noexcept override
  // {
  //   if (auto iter =
  //         doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
  //       iter != doc_.MemberEnd() && iter->value.IsString()) {
  //     return { iter->value.GetString(), iter->value.GetStringLength() };
  //   } else {
  //     return default_value;
  //   }
  // }

  // int32_t get_int(std::string_view name) const override
  // {
  //   if (auto iter =
  //         doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
  //       iter != doc_.MemberEnd() && iter->value.IsInt()) {
  //     return iter->value.GetInt();
  //   } else {
  //     throw NoKeyException(name);
  //   }
  // }

  // std::string_view get_string(std::string_view name) const override
  // {
  //   if (auto iter =
  //         doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
  //       iter != doc_.MemberEnd() && iter->value.IsString()) {
  //     return { iter->value.GetString(), iter->value.GetStringLength() };
  //   } else {
  //     throw NoKeyException(name);
  //   }
  // }

  // void set_value(std::string_view name, int32_t value) override
  // {
  //   if (auto iter =
  //         doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
  //       iter != doc_.MemberEnd()) {
  //     iter->value.SetInt(value);
  //   } else {
  //     doc_.AddMember(rapidjson::StringRef(name.data(), name.size()),
  //                    rapidjson::Value(value),
  //                    doc_.GetAllocator());
  //   }
  // }

  // void set_value(std::string_view name, std::string_view value) override
  // {
  //   if (auto iter =
  //         doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
  //       iter != doc_.MemberEnd()) {
  //     iter->value.SetString(value.data(), value.size());
  //   } else {
  //     doc_.AddMember(rapidjson::StringRef(name.data(), name.size()),
  //                    rapidjson::StringRef(value.data(), value.size()),
  //                    doc_.GetAllocator());
  //   }
  // }

  Node& get_root(/* std::string_view name */) override
  {
    // auto value =
    //   doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
    // assert(value != doc_.MemberEnd());
    return root_;
  }

  const Node& get_root(/* std::string_view name */) const override
  {
    // auto value =
    //   doc_.FindMember(rapidjson::StringRef(name.data(), name.size()));
    // assert(value != doc_.MemberEnd());
    return root_;
  }

  std::string to_string() const override { return {}; }

  std::string to_binary() const override { return {}; }

  void from_string(std::string_view str) override {}

  void from_binary(std::string_view bin) override {}

  void clear() override { doc_.RemoveAllMembers(); }

private:
  rapidjson::Document doc_;
  JsonNode root_;
};
}
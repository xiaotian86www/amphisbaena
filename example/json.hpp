#include <rapidjson/document.h>

#include "translator.hpp"

class JsonObject
    : public translator::Object
{
public:
    JsonObject()
        : obj_(rapidjson::kObjectType) {}

public:
    int32_t get_value(
        std::string_view name, int32_t default_value) const override
    {
        auto iter = obj_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        if (iter == obj_.MemberEnd())
            return default_value;
        else
            return iter->value.GetInt();
    }

    std::string_view get_value(
        std::string_view name, std::string_view default_value) const override
    {
        auto iter = obj_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        if (iter == obj_.MemberEnd())
            return default_value;
        else
            return { iter->value.GetString(), iter->value.GetStringLength() };
    }

    void set_value(
        std::string_view name, int32_t value) override
    {
        auto iter = obj_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        if (iter == obj_.MemberEnd())
            obj_.AddMember(rapidjson::Value(name.data(), name.size(), obj_.GetAllocator()),
                           rapidjson::Value(value), obj_.GetAllocator());
        else
            iter->value.SetInt(value);
    }

    void set_value(
        std::string_view name, std::string_view value) override
    {
        auto iter = obj_.FindMember(rapidjson::StringRef(name.data(), name.size()));
        if (iter == obj_.MemberEnd())
            obj_.AddMember(rapidjson::Value(name.data(), name.size(), obj_.GetAllocator()),
                           rapidjson::Value(value.data(), value.size()), obj_.GetAllocator());
        else
            iter->value.SetString(value.data(), value.size(), obj_.GetAllocator());
    }

private:
    rapidjson::Document obj_;
};

#include <rapidjson/document.h>

#include "translator.hpp"

template <>
class Object<rapidjson::Document>
{
public:
    Object(rapidjson::Document &obj)
        : obj_(obj) {}

public:
    int32_t get_value(
        const char *name, int32_t default_value) const
    {
        auto iter = obj_.FindMember(name);
        if (iter == obj_.MemberEnd())
            return default_value;
        else
            return iter->value.GetInt();
    }

    void set_value(
        const char *name, int32_t value)
    {
        auto iter = obj_.FindMember(name);
        if (iter == obj_.MemberEnd())
            obj_.AddMember(rapidjson::Value(name, obj_.GetAllocator()),
                           rapidjson::Value(value), obj_.GetAllocator());
        else
            iter->value.SetInt(value);
    }

private:
    rapidjson::Document &obj_;
};

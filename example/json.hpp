#pragma once

#include <rapidjson/document.h>

#include "object.hpp"

namespace detail
{
    class JsonObject
        : public translator::Object
    {
    protected:
        void set_object(rapidjson::Value *obj)
        {
            obj_ = obj;
        }

        void set_allocator(rapidjson::Value::AllocatorType *allocator)
        {
            allocator_ = allocator;
        }

    public:
        int32_t get_value(
            std::string_view name, int32_t default_value) const override
        {
            auto iter = obj_->FindMember(rapidjson::StringRef(name.data(), name.size()));
            if (iter == obj_->MemberEnd())
                return default_value;
            else
                return iter->value.GetInt();
        }

        std::string_view get_value(
            std::string_view name, std::string_view default_value) const override
        {
            auto iter = obj_->FindMember(rapidjson::StringRef(name.data(), name.size()));
            if (iter == obj_->MemberEnd())
                return default_value;
            else
                return {iter->value.GetString(), iter->value.GetStringLength()};
        }

        void set_value(
            std::string_view name, int32_t value) override
        {
            auto iter = obj_->FindMember(rapidjson::StringRef(name.data(), name.size()));
            if (iter == obj_->MemberEnd())
                obj_->AddMember(rapidjson::Value(name.data(), name.size(), *allocator_),
                                rapidjson::Value(value), *allocator_);
            else
                iter->value.SetInt(value);
        }

        void set_value(
            std::string_view name, std::string_view value) override
        {
            auto iter = obj_->FindMember(rapidjson::StringRef(name.data(), name.size()));
            if (iter == obj_->MemberEnd())
                obj_->AddMember(rapidjson::Value(name.data(), name.size(), *allocator_),
                                rapidjson::Value(value.data(), value.size()), *allocator_);
            else
                iter->value.SetString(value.data(), value.size(), *allocator_);
        }

        translator::Group *get_group(std::string_view name) override
        {
            return nullptr;
        }

        const translator::Group *get_group(std::string_view name) const override
        {
            return nullptr;
        }

    private:
        rapidjson::Value *obj_ = nullptr;
        rapidjson::Value::AllocatorType *allocator_ = nullptr;
    };

} // namespace detail

class JsonDocument
    : public detail::JsonObject
{
public:
    JsonDocument()
        : document_(rapidjson::kObjectType)
    {
        set_object(&document_);
        set_allocator(&document_.GetAllocator());
    }

public:
    rapidjson::Document document_;
};

#pragma once

#include <cstdint>
#include <string_view>
#include <functional>

#include <boost/lexical_cast.hpp>

namespace translator
{
    class Object
    {
    public:
        virtual ~Object() = default;

    public:
        virtual int32_t get_value(
            std::string_view name, int32_t default_value) const = 0;

        virtual std::string_view get_value(
            std::string_view name, std::string_view default_value) const = 0;

        virtual void set_value(std::string_view name, int32_t value) = 0;

        virtual void set_value(std::string_view name, std::string_view value) = 0;

        template <typename GroupType_>
        GroupType_ &get_group(std::string_view name)
        {
        }

        template <typename GroupType_>
        const GroupType_ &get_group(std::string_view name) const
        {
        }
    };

    class Group
    {
    public:
        virtual ~Group() = default;

    public:
        virtual Object *at(std::size_t index) = 0;

        virtual const Object *at(std::size_t index) const = 0;

        virtual std::size_t get_size() const = 0;

        template <typename IterType_>
        IterType_ begin();

        template <typename IterType_>
        IterType_ end();

        template <typename ConstIterType_>
        ConstIterType_ begin() const;

        template <typename ConstIterType_>
        ConstIterType_ end() const;
    };

} // namespace translator

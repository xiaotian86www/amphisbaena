#pragma once

#include "object.hpp"
#include "context.hpp"

namespace translator
{
    template <typename ValueType_>
    using get_value_func_t = std::function<ValueType_(Context *)>;

    using set_value_func_t = std::function<void(Context *, Object *)>;

    template <typename ValueType_>
    get_value_func_t<ValueType_> get_value(
        std::string_view object_name,
        std::string_view field_name, ValueType_ default_value)
    {
        return [object_name, field_name, default_value](Context *context)
        {
            auto obj = context->get_object(object_name);
            return obj->get_value(field_name, default_value);
        };
    }

    template <typename Target_, typename Source_>
    get_value_func_t<Target_> lexical_cast(
        get_value_func_t<Source_> &&func)
    {
        return [func = std::move(func)](Context *context)
        {
            return boost::lexical_cast<Target_>(func(context));
        };
    }

    template <typename ValueType_>
    get_value_func_t<ValueType_> add(
        get_value_func_t<ValueType_> &&left,
        get_value_func_t<ValueType_> &&right)
    {
        return [left = std::move(left), right = std::move(right)](Context *context)
        {
            return left(context) + right(context);
        };
    }

    template <typename ValueType_>
    get_value_func_t<ValueType_> sub(
        get_value_func_t<ValueType_> &&left,
        get_value_func_t<ValueType_> &&right)
    {
        return [left = std::move(left), right = std::move(right)](Context *context)
        {
            return left(context) - right(context);
        };
    }

    template <typename ValueType_>
    get_value_func_t<ValueType_> make_value(ValueType_ value)
    {
        return [value](Context *context)
        {
            return value;
        };
    }

    template <typename ValueType_>
    set_value_func_t set_value(std::string_view name, get_value_func_t<ValueType_> &&func)
    {
        return [name, func = std::move(func)](Context *context, Object *obj)
        {
            obj->set_value(name, func(context));
        };
    }

} // namespace translator

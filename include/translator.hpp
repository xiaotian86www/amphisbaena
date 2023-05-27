#pragma once

#include <cstdint>
#include <string_view>
#include <functional>
#include <map>
#include <sstream>

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

    class Context
    {
    public:
        using get_object_func_t = std::function<std::unique_ptr<Object>(Context *)>;

    public:
        Object *get_object(std::string_view name)
        {
            auto it = objects_.find(name);
            if (it != objects_.end())
                return it->second.get();

            auto func_it = object_funcs_.find(name);
            if (func_it != object_funcs_.end())
            {
                auto obj = func_it->second(this);
                auto it = objects_.insert(std::make_pair(name, std::move(obj)));
                return it.first->second.get();
            }

            std::stringstream ss;
            ss << "cannot find object '" << name << "'";

            throw new std::runtime_error(ss.str());
        }

        void set_object(std::string_view name, std::unique_ptr<Object>&& obj)
        {
            auto it = objects_.find(name);
            if (it != objects_.end())
                it->second = std::move(obj);
            else
                objects_.insert(std::make_pair(name, std::move(obj)));
        }

        void reset()
        {
            objects_.clear();
        }

        void set_object_func(std::string_view name, get_object_func_t func)
        {
            object_funcs_.insert_or_assign(name, func);
        }

    private:
        std::map<std::string_view, std::unique_ptr<Object>> objects_;
        std::map<std::string_view, get_object_func_t> object_funcs_;
    };

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

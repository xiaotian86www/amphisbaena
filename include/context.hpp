#pragma once

#include <map>
#include <sstream>
#include <functional>

extern "C"
{
#include <coroutine/coroutine.h>
}

#include "object.hpp"
#include "server.hpp"

namespace translator
{
    struct task
    {
    };

    class Context
    {
    public:
        using get_object_func_t = std::function<std::unique_ptr<Object>(Context *)>;

    public:
        Context(schedule *s, int32_t co_id)
            : s_(s), co_id_(co_id)
        {
        }

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

        void set_object(std::string_view name, std::unique_ptr<Object> &&obj)
        {
            auto it = objects_.find(name);
            if (it != objects_.end())
                it->second = std::move(obj);
            else
                objects_.insert(std::make_pair(name, std::move(obj)));
        }

        void set_object_func(std::string_view name, get_object_func_t func)
        {
            object_funcs_.insert_or_assign(name, func);
        }

        void set_server(std::string_view name, std::unique_ptr<Server> &&server)
        {
            auto it = servers_.find(name);
            if (it != servers_.end())
                it->second = std::move(server);
            else
                servers_.insert(std::make_pair(name, std::move(server)));
        }

        Server *get_server(std::string_view name)
        {
            auto it = servers_.find(name);
            if (it != servers_.end())
                return it->second.get();

            std::stringstream ss;
            ss << "cannot find server '" << name << "'";

            throw new std::runtime_error(ss.str());
        }

        void reset()
        {
            objects_.clear();
        }

        void yield()
        {
            coroutine_yield(s_);
        }

    public:
        int32_t co_id()
        {
            return co_id_;
        }

        void set_task(task &&t)
        {
            t_ = std::move(t);
        }

    private:
        int32_t co_id_;
        std::map<std::string_view, std::unique_ptr<Object>> objects_;
        std::map<std::string_view, get_object_func_t> object_funcs_;
        std::map<std::string_view, std::unique_ptr<Server>> servers_;
        task t_;
        schedule *s_;
    };

} // namespace translator

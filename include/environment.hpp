#pragma once

#include <memory>
#include <unordered_map>

#include "object.hpp"

namespace translator
{
    class Environment
    {
    public:
        Environment(ObjectFactoryPtr object_factory)
            : object_factory_(object_factory)
        {
        }

    public:
        Object *get_object(std::string_view name);

        void set_object(std::string_view name, ObjectPtr &&object);

    private:
        std::unordered_map<std::string_view, ObjectPtr> objects_;
        ObjectFactoryPtr object_factory_;
    };
} // namespace translator

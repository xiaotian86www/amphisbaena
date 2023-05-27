#pragma once

#include "object.hpp"

namespace translator
{
    class Server
    {
    public:
        virtual ~Server() = default;

        virtual std::unique_ptr<Object> call(
            std::string_view method, const Object *args) = 0;
    };
} // namespace translator

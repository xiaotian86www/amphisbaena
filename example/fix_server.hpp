#pragma once

#include "server.hpp"
#include "json.hpp"

class FixServer
    : public translator::Server
{
public:
    std::unique_ptr<translator::Message> call(
        std::string_view method, const translator::Message *args) override
    {
        auto rsp = std::make_unique<JsonDocument>();
        rsp->set_value("field1", args->get_value("field1", ""));
        rsp->set_value("field2", args->get_value("field2", ""));

        return rsp;
    }
};
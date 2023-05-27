#include <iostream>

#include "json.hpp"
#include "fix_server.hpp"
#include "func.hpp"

int main(int argc, char **argv)
{
    translator::Context context;

    context.set_server("fix", std::make_unique<FixServer>());

    auto trans_field1 = translator::set_value(
        "field1", translator::add(
                      translator::lexical_cast<int32_t>(
                          translator::get_value<std::string_view>("obj3", "field1", "0")),
                      translator::lexical_cast<int32_t>(
                          translator::get_value<std::string_view>("obj3", "field2", "0"))));
    auto trans_field2 = translator::set_value(
        "field2", translator::sub(
                      translator::lexical_cast<int32_t>(
                          translator::get_value<std::string_view>("obj3", "field1", "0")),
                      translator::lexical_cast<int32_t>(
                          translator::get_value<std::string_view>("obj3", "field2", "0"))));

    context.set_object_func("obj1", [](translator::Context *context)
                            {
                                auto obj = std::make_unique<JsonDocument>();
                                obj->set_value("field1", "2");
                                obj->set_value("field2", "1");
                                return obj; });

    context.set_object_func("obj3", [](translator::Context *context){
        auto server = context->get_server("fix");
        auto arg = context->get_object("obj1");
        return server->call("get", arg);
    });

    context.set_object_func("obj2", [&trans_field1, &trans_field2](translator::Context *context)
                            {
                                auto obj = std::make_unique<JsonDocument>();
                                
                                trans_field1(context, obj.get());
                                trans_field2(context, obj.get());
                                return obj; });

    std::cout << "field1: " << context.get_object("obj2")->get_value("field1", 0) << ", "
        << "field2: " << context.get_object("obj2")->get_value("field2", 0) << std::endl;
    return 0;
}
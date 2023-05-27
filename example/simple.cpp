#include <iostream>

#include "json.hpp"

int main(int argc, char** argv)
{
    rapidjson::Document obj(rapidjson::kObjectType);
    Object<rapidjson::Document> obj_wrapper(obj);

    obj_wrapper.set_value("field1", 1);
    std::cout << "field1: " << obj_wrapper.get_value("field1", 0) << std::endl;
    return 0;
}
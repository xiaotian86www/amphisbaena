
#include <memory>

#include "builder.hpp"
#include "environment.hpp"
#include "impl/json_message.hpp"
#include "message.hpp"

extern "C"
{
  void* get_func2()
  {
    return new translator::MessageBuilder::ctor_function(
      [](translator::Environment&, translator::MessagePtr) {
        auto message = std::make_shared<translator::JsonMessage>();
        message->get_body()->set_value("Field", 1);
        return message;
      });
  }

  const char* get_name2()
  {
    return "Message";
  }
}
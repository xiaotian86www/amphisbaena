
#include <memory>

#include "impl/fix_message.hpp"
#include "impl/json_message.hpp"
#include "message.hpp"
#include "log.hpp"

namespace translator {

void
Object::copy_from(ObjectPtr right)
{
  if (this == right.get())
    return;

  for (auto iter = right->begin(); iter != right->end(); ++iter) {
    switch (iter.get_type()) {
      case FieldType::kString:
        set_value(iter.get_name(), iter.get_string());
        break;
      case FieldType::kInt:
        set_value(iter.get_name(), iter.get_int());
        break;
      case FieldType::kDouble:
        set_value(iter.get_name(), iter.get_double());
        break;
      case FieldType::kUnknown:
        break;
    }
  }
}

void
MessageFactory::registe(std::string_view type, ctor_function ctor)
{
  LOG_INFO("Registe message type: {}", type);
  auto ctors =
    ctors_
      ? std::make_shared<std::map<std::string, ctor_function, std::less<>>>(
          *ctors_)
      : std::make_shared<std::map<std::string, ctor_function, std::less<>>>();
  ctors->insert_or_assign(std::string(type), std::move(ctor));

  ctors_ = ctors;
}

void
MessageFactory::unregiste()
{
  LOG_INFO("Unregiste message");
  ctors_ =
    std::make_shared<std::map<std::string, ctor_function, std::less<>>>();
}

MessagePtr
MessageFactory::create(std::string_view type)
{
  if (auto ctors = ctors_) {
    if (auto iter = ctors->find(type); iter != ctors->end()) {
      return iter->second();
    }
  }

  return MessagePtr();
}

std::shared_ptr<
  std::map<std::string, MessageFactory::ctor_function, std::less<>>>
  MessageFactory::ctors_;
} // namespace translator


#include <memory>

#include "log.hpp"
#include "message.hpp"

namespace amphisbaena {

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
MessageFactory::registe(std::shared_ptr<MessageFactory> factory)
{
  LOG_INFO("Registe message type: {}", factory->name());
  auto ctors =
    ctors_
      ? std::make_shared<
          std::map<std::string, std::shared_ptr<MessageFactory>, std::less<>>>(
          *ctors_)
      : std::make_shared<std::map<std::string,
                                  std::shared_ptr<MessageFactory>,
                                  std::less<>>>();
  ctors->insert_or_assign(std::string(factory->name()), factory);

  ctors_ = ctors;
}

void
MessageFactory::unregiste()
{
  LOG_INFO("Unregiste message");
  ctors_ = std::make_shared<
    std::map<std::string, std::shared_ptr<MessageFactory>, std::less<>>>();
}

void
MessageFactory::unregiste(std::shared_ptr<MessageFactory> factory)
{
  LOG_INFO("Unregiste message type: {}", factory->name());
  auto ctors =
    ctors_
      ? std::make_shared<std::map<std::string, std::shared_ptr<MessageFactory>, std::less<>>>(
          *ctors_)
      : std::make_shared<std::map<std::string, std::shared_ptr<MessageFactory>, std::less<>>>();
  ctors->erase(std::string(factory->name()));

  ctors_ = ctors;
}

MessagePtr
MessageFactory::create(std::string_view type)
{
  if (auto ctors = ctors_) {
    if (auto iter = ctors->find(type); iter != ctors->end()) {
      return iter->second->create();
    }
  }

  return MessagePtr();
}

std::shared_ptr<
  std::map<std::string, std::shared_ptr<MessageFactory>, std::less<>>>
  MessageFactory::ctors_;
} // namespace amphisbaena

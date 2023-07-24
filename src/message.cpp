
#include <memory>

#include "impl/fix_message.hpp"
#include "impl/json_message.hpp"
#include "message.hpp"

namespace translator {

void
Object::copy_from(ObjectPtr right)
{
  if (this == right.get()) return;

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

MessagePtr
MessageFactory::create(MessageType type)
{
  switch (type) {
    case MessageType::kJson:
      return std::make_shared<JsonMessage>();
    case MessageType::kFix:
      return std::make_shared<FixMessage>();
  }
}
} // namespace translator


#include <memory>

#include "impl/fix_message.hpp"
#include "impl/json_message.hpp"
#include "message.hpp"

namespace translator {
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

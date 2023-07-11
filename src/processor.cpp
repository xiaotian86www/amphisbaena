#include "processor.hpp"
#include <memory>

namespace translator {
std::shared_ptr<Processor>
ProcessorFactory::create(std::string_view key)
{
  if (auto iter = ctors_.find(key); iter != ctors_.end()) {
    return iter->second();
  } else {
    return {};
  }
}

void
ProcessorFactory::registe(std::string_view key, ctor_function ctor)
{
  ctors_[key] = std::move(ctor);
}

void
ProcessorFactory::unregiste(std::string_view key)
{
  ctors_.erase(key);
}
}
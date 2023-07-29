#include "builder.hpp"

#include <dlfcn.h>
#include <map>
#include <memory>
#include <sstream>
#include <string_view>

#include "environment.hpp"
#include "message.hpp"

namespace translator {
void
MessageBuilder::registe(
  std::map<std::string_view, std::shared_ptr<MessageBuilder>> builders)
{
  auto new_builders =
    builders_
      ? std::make_shared<
          std::map<std::string, std::shared_ptr<MessageBuilder>, std::less<>>>(
          *builders_)
      : std::make_shared<std::map<std::string,
                                  std::shared_ptr<MessageBuilder>,
                                  std::less<>>>();

  for (auto iter = builders.begin(); iter != builders.end(); ++iter) {
    new_builders->insert_or_assign(std::string(iter->first),
                                   std::move(iter->second));
  }

  builders_ = new_builders;
}

void
MessageBuilder::unregiste()
{
  builders_ = std::make_shared<
    std::map<std::string, std::shared_ptr<MessageBuilder>, std::less<>>>();
}

void
MessageBuilder::unregiste(const std::vector<std::string_view>& names)
{
  auto new_ctors =
    builders_
      ? std::make_shared<
          std::map<std::string, std::shared_ptr<MessageBuilder>, std::less<>>>(
          *builders_)
      : std::make_shared<std::map<std::string,
                                  std::shared_ptr<MessageBuilder>,
                                  std::less<>>>();

  for (auto it = names.begin(); it != names.end(); ++it) {
    new_ctors->erase(std::string(*it));
  }

  builders_ = new_ctors;
}

MessagePtr
MessageBuilder::create(Environment& env,
                       std::string_view name,
                       MessagePtr request)
{
  if (auto builders = builders_) {
    if (auto it = builders->find(name); it != builders->end()) {
      return it->second->create(env, request);
    }
  }

  throw NotFoundException(name);
}

// std::unordered_map<std::string_view, std::shared_ptr<MessageBuilder>>
//   MessageBuilder::ctors_;
std::shared_ptr<
  std::map<std::string, std::shared_ptr<MessageBuilder>, std::less<>>>
  MessageBuilder::builders_;

void
Plugin::load(const std::vector<std::filesystem::path>& paths)
{
  std::map<std::string_view, std::shared_ptr<MessageBuilder>> builders;
  for (const auto& path : paths) {
    auto handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!handle)
      throw CouldnotLoadException(path.string(), dlerror());

    MessageBuilder* (*get_builder)() = nullptr;
    const char* (*get_name)() = nullptr;

    *(void**)(&get_builder) = dlsym(handle, "get_builder");
    if (!get_builder)
      throw CouldnotLoadException(path.string(), dlerror());

    *(void**)(&get_name) = dlsym(handle, "get_name");
    if (!get_name)
      throw CouldnotLoadException(path.string(), dlerror());

    builders.insert_or_assign(
      (*get_name)(), std::shared_ptr<MessageBuilder>((*get_builder)()));
  }

  MessageBuilder::registe(std::move(builders));
}
}
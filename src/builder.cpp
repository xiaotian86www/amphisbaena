
#include <dlfcn.h>
#include <map>
#include <memory>
#include <sstream>
#include <string_view>
#include <tuple>
#include <utility>

#include "builder.hpp"
#include "environment.hpp"
#include "log.hpp"
#include "message.hpp"

namespace amphisbaena {
void
MessageBuilder::registe(std::shared_ptr<MessageBuilder> builder)
{
  LOG_INFO("Registe pattern: {}", builder->name());
  auto new_builders =
    builders_
      ? std::make_shared<
          std::map<std::string, std::shared_ptr<MessageBuilder>, std::less<>>>(
          *builders_)
      : std::make_shared<std::map<std::string,
                                  std::shared_ptr<MessageBuilder>,
                                  std::less<>>>();

  new_builders->insert_or_assign(std::string(builder->name()), builder);

  builders_ = new_builders;
}

void
MessageBuilder::unregiste()
{
  LOG_INFO("Unregiste pattern");
  builders_ = std::make_shared<
    std::map<std::string, std::shared_ptr<MessageBuilder>, std::less<>>>();
}

void
MessageBuilder::unregiste(std::shared_ptr<MessageBuilder> builder)
{
  LOG_INFO("Unregiste pattern: {}", builder->name());
  if (auto iter = builders_->find(builder->name());
      iter != builders_->end() && iter->second == builder) {
    auto new_builders =
      builders_ ? std::make_shared<std::map<std::string,
                                            std::shared_ptr<MessageBuilder>,
                                            std::less<>>>(*builders_)
                : std::make_shared<std::map<std::string,
                                            std::shared_ptr<MessageBuilder>,
                                            std::less<>>>();

    new_builders->erase(std::string(builder->name()));

    builders_ = new_builders;
  }
}

MessagePtr
MessageBuilder::create(Environment& env,
                       std::string_view pattern,
                       MessagePtr request)
{
  LOG_DEBUG("pattern: {}", pattern);
  if (auto builders = builders_) {
    if (auto it = builders->find(pattern); it != builders->end()) {
      return it->second->create(env, request);
    }
  }

  throw NotFoundException(pattern);
}

// std::unordered_map<std::string_view, std::shared_ptr<MessageBuilder>>
//   MessageBuilder::ctors_;
std::shared_ptr<
  std::map<std::string, std::shared_ptr<MessageBuilder>, std::less<>>>
  MessageBuilder::builders_;

// void
// Plugin::load(
//   const std::map<std::filesystem::path, std::vector<std::string>>& infos)
// {
//   for (const auto& info : infos) {
//     load(info.first, info.second);
//   }
// }

// void
// Plugin::load(const std::filesystem::path& path)
// {
//   load(path, {});
// }

// void
// Plugin::load(const std::filesystem::path& path,
//              const std::vector<std::string>& args)
// {
//   auto handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
//   if (!handle)
//     throw CouldnotLoadException(path.string(), dlerror());

//   void (*init)(int, const char**) = nullptr;
//   *(void**)(&init) = dlsym(handle, "init");
//   if (!init)
//     throw CouldnotLoadException(path.string(), dlerror());

//   std::vector<const char*> args_;
//   args_.push_back(path.c_str());
//   for (const auto& arg : args) {
//     args_.push_back(arg.c_str());
//   }

//   init(args_.size(), args_.data());
// }
}
#include "builder.hpp"

#include <dlfcn.h>
#include <memory>
#include <sstream>
#include <string_view>

#include "environment.hpp"
#include "message.hpp"

namespace translator {
namespace detail {
class Plugin
{
public:
  template<typename Deleter_>
  Plugin(void* handle, Deleter_ d, MessageBuilder::ctor_function* ctor)
    : handle_(handle, std::move(d))
    , ctor_(ctor)
  {
  }

  ~Plugin() {}

public:
  MessagePtr operator()(Environment& env, MessagePtr request)
  {
    return (*ctor_)(env, request);
  }

private:
  std::shared_ptr<void> handle_;
  std::shared_ptr<MessageBuilder::ctor_function> ctor_;
};
}

void
MessageBuilder::registe(std::map<std::string_view, ctor_function> ctors)
{
  auto new_ctors =
    ctors_
      ? std::make_shared<
          std::map<std::string, MessageBuilder::ctor_function, std::less<>>>(
          *ctors_)
      : std::make_shared<
          std::map<std::string, MessageBuilder::ctor_function, std::less<>>>();

  for (auto iter = ctors.begin(); iter != ctors.end(); ++iter) {
    new_ctors->insert_or_assign(std::string(iter->first),
                                std::move(iter->second));
  }

  ctors_ = new_ctors;
}

void
MessageBuilder::unregiste()
{
  ctors_ = std::make_shared<
    std::map<std::string, MessageBuilder::ctor_function, std::less<>>>();
}

void
MessageBuilder::unregiste(const std::vector<std::string_view>& names)
{
  auto new_ctors =
    ctors_
      ? std::make_shared<
          std::map<std::string, MessageBuilder::ctor_function, std::less<>>>(
          *ctors_)
      : std::make_shared<
          std::map<std::string, MessageBuilder::ctor_function, std::less<>>>();

  for (auto it = names.begin(); it != names.end(); ++it) {
    new_ctors->erase(std::string(*it));
  }

  ctors_ = new_ctors;
}

MessagePtr
MessageBuilder::create(Environment& env,
                       std::string_view name,
                       MessagePtr request)
{
  if (auto ctors = ctors_) {
    if (auto it = ctors->find(name); it != ctors->end()) {
      return it->second(env, request);
    }
  }

  throw NotFoundException(name);
}

// std::unordered_map<std::string_view, MessageBuilder::ctor_function>
//   MessageBuilder::ctors_;
std::shared_ptr<
  std::map<std::string, MessageBuilder::ctor_function, std::less<>>>
  MessageBuilder::ctors_;

void
Plugin::load(const std::vector<std::filesystem::path>& paths)
{
  std::map<std::string_view, MessageBuilder::ctor_function> ctors;
  for (const auto& path : paths) {
    auto handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!handle)
      throw CouldnotLoadException(path.string(), dlerror());

    void* (*get_func)() = nullptr;
    const char* (*get_name)() = nullptr;

    *(void**)(&get_func) = dlsym(handle, "get_func");
    if (!get_func)
      throw CouldnotLoadException(path.string(), dlerror());

    *(void**)(&get_name) = dlsym(handle, "get_name");
    if (!get_name)
      throw CouldnotLoadException(path.string(), dlerror());

    ctors.insert_or_assign(
      (*get_name)(),
      detail::Plugin(
        handle,
        [](void* h) { dlclose(h); },
        static_cast<MessageBuilder::ctor_function*>((*get_func)())));
  }

  MessageBuilder::registe(std::move(ctors));
}
}
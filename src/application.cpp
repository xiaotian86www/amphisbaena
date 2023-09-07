#include <memory>

#include "application.hpp"
#include "exception.hpp"

namespace amphisbaena {

Application::~Application()
{
  // 先插入的后销毁
  while (!plugins_.empty()) {
    plugins_.pop_back();
  }
}

Application&
Application::get_instance()
{
  static Application instance;
  return instance;
}

void
Application::load(std::string_view name,
                  const std::filesystem::path& path,
                  const std::vector<std::string>& args)
{
  auto iter = get(name);
  if (iter != plugins_.end()) {
    throw PluginExistedException(name);
  } else {
    plugins_.push_back(std::make_unique<Plugin>(name, path, args));
  }
}

void
Application::reload(std::string_view name,
                    const std::filesystem::path& path,
                    const std::vector<std::string>& args)
{
  auto iter = get(name);
  if (iter == plugins_.end()) {
    throw PluginNotFoundException(name);
  } else {
    *iter = std::make_unique<Plugin>(name, path, args);
  }
}

void
Application::unload(std::string_view name)
{
  auto iter = get(name);
  if (iter == plugins_.end()) {
    throw PluginNotFoundException(name);
  } else {
    plugins_.erase(iter);
  }
}

std::vector<std::unique_ptr<Plugin>>::iterator
Application::get(std::string_view name)
{
  auto iter = plugins_.begin();
  for (; iter != plugins_.end(); ++iter) {
    if ((*iter)->name() == name) {
      return iter;
    }
  }

  return iter;
}

}
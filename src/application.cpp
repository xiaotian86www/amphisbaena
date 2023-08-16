#include <memory>

#include "application.hpp"

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
  auto& plugin = get(name);
  if (plugin) {
    // TODO 抛出异常
  } else {
    plugins_.push_back(std::make_unique<Plugin>(name, path, args));
  }
}

void
Application::reload(std::string_view name,
                    const std::filesystem::path& path,
                    const std::vector<std::string>& args)
{
  auto& plugin = get(name);
  if (!plugin) {
    // TODO 抛出异常
  } else {
    plugin = std::make_unique<Plugin>(name, path, args);
  }
}

std::unique_ptr<Plugin>&
Application::get(std::string_view name)
{
  static std::unique_ptr<Plugin> empty;

  for (auto& plugin : plugins_) {
    if (plugin->name() == name) {
      return plugin;
    }
  }

  return empty;
}

}
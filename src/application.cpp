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

bool
Application::load(std::string_view name,
                  const std::filesystem::path& path,
                  const std::vector<std::string>& args)
{
  auto iter = get(name);
  if (iter != plugins_.end())
    return false;

  plugins_.push_back(std::make_unique<Plugin>(name, path, args));
  return true;
}

bool
Application::reload(std::string_view name,
                    const std::filesystem::path& path,
                    const std::vector<std::string>& args)
{
  auto iter = get(name);
  if (iter == plugins_.end())
    return false;

  *iter = std::make_unique<Plugin>(name, path, args);
  return true;
}

bool
Application::unload(std::string_view name)
{
  auto iter = get(name);
  if (iter == plugins_.end())
    return false;

  plugins_.erase(iter);
  return true;
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
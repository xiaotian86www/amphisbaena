#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "plugin.hpp"

namespace amphisbaena {
class Application
{
private:
  Application() = default;
  ~Application();

public:
  static Application& get_instance();

public:
  void load(std::string_view name,
            const std::filesystem::path& path,
            const std::vector<std::string>& args);

  void reload(std::string_view name,
              const std::filesystem::path& path,
              const std::vector<std::string>& args);

private:
  std::unique_ptr<Plugin>& get(std::string_view name);

private:
  std::vector<std::unique_ptr<Plugin>> plugins_;
};
}
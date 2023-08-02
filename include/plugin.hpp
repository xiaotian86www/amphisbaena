#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace amphisbaena {
class Plugin
{
public:
  Plugin(const std::filesystem::path& path);

  Plugin(const std::filesystem::path& path,
         const std::vector<std::string>& args);

  ~Plugin();

private:
  void init(const std::vector<const char*>& args);

  void deinit();

private:
  std::filesystem::path path_;
  void* handle_;
};
}
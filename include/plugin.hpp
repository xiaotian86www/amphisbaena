#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace amphisbaena {
class Plugin
{
public:
  Plugin(std::string_view name,
         const std::filesystem::path& path,
         const std::vector<std::string>& args);

  ~Plugin();

  // TODO 新增plugin容器管理功能

public:
  std::string_view name() { return name_; }

private:
  void init(const std::vector<const char*>& args);

  void deinit();

private:
  std::string name_;
  std::filesystem::path path_;
  void* handle_;
};
}
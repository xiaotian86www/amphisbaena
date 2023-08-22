/**
 * @file application.hpp
 * @author duchang (xiaotian86www@163.com)
 * @brief 
 * @version 0.1
 * @date 2023-08-22
 * 
 * @copyright Copyright (c) 2023
 * 
 */

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
/**
 * @brief 应用类
 * 
 * 单例类，管理插件生命周期，提供load和reload方法，实现运行时插件加载和重加载。
 */
class Application
{
private:
  Application() = default;
  ~Application();

public:
  static Application& get_instance();

public:
  /**
   * @brief 加载插件
   * 
   * @param name 插件名，全局唯一
   * @param path 插件库路径
   * @param args 插件初始化参数
   */
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
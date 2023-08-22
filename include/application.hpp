/**
 * @file application.hpp
 * @author duchang (xiaotian86www@163.com)
 * @brief 应用类
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
 * 单例类，管理插件生命周期。
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
   * @brief 加载插件，先加载的插件在不主动卸载的情况下后释放，
   * 这样保证了插件之间正确的依赖关系
   * 
   * @param[in] name 插件名，全局唯一
   * @param[in] path 插件库路径
   * @param[in] args 插件初始化参数
   *
   * @exception PluginNameExitsException 插件名已存在
   */
  void load(std::string_view name,
            const std::filesystem::path& path,
            const std::vector<std::string>& args);

  /**
   * @brief 重新加载插件，不会改变插件的加载顺序和释放顺序
   * 
   * @param[in] name 插件名，全局唯一
   * @param[in] path 插件库路径
   * @param[in] args 插件初始化参数
   *
   * @exception PluginNameNotExitsException 插件名不存在
   */
  void reload(std::string_view name,
              const std::filesystem::path& path,
              const std::vector<std::string>& args);

private:
  std::unique_ptr<Plugin>& get(std::string_view name);

private:
  std::vector<std::unique_ptr<Plugin>> plugins_;
};
}
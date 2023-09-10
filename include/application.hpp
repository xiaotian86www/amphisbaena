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
   * @retval true 加载成功
   * @retval false 加载失败
   */
  bool load(std::string_view name,
            const std::filesystem::path& path,
            const std::vector<std::string>& args);

  /**
   * @brief 重新加载插件，不会改变插件的加载顺序和释放顺序
   *
   * @param[in] name 插件名，全局唯一
   * @param[in] path 插件库路径
   * @param[in] args 插件初始化参数
   *
   * @retval true 加载成功
   * @retval false 加载失败
   */
  bool reload(std::string_view name,
              const std::filesystem::path& path,
              const std::vector<std::string>& args);

  /**
   * @brief 卸载插件
   *
   * @param name 插件名，全局唯一
   *
   * @retval true 卸载成功
   * @retval false 卸载失败
   */
  bool unload(std::string_view name);

  /**
   * @brief 卸载所有插件
   *
   */
  void unload();

private:
  std::vector<std::unique_ptr<Plugin>>::iterator get(std::string_view name);

private:
  std::vector<std::unique_ptr<Plugin>> plugins_;
};
}
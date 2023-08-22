/**
 * @file plugin.hpp
 * @author duchang (xiaotian86www@163.com)
 * @brief 插件类
 * @version 0.1
 * @date 2023-08-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace amphisbaena {
/**
 * @brief 插件类，封装了插件的加载和卸载行为
 *
 * 如何定义一个插件？
 * 
 * - 需要将插件打包成一个动态库
 * - 定义一个全局方法 @code {.c} void init(const char* const* args) @endcode，
 * 并实现初始化逻辑，会在插件加载后调用
 * - 定义一个全局方法（可选） @code {.c} void deinit() @endcode，
 * 并实现析构逻辑，会在插件卸载前调用
 */
class Plugin
{
public:
  /**
   * @brief 构造函数，使用dlopen打开插件库，并调用插件的init方法
   * 
   * @param[in] name 插件名，全局唯一
   * @param[in] path 插件库路径
   * @param[in] args 插件初始化参数
   *
   * @exception CouldnotLoadException 不能打开插件，
   * 当插件不存在或不存在init方法时，抛出该异常
   */
  Plugin(std::string_view name,
         const std::filesystem::path& path,
         const std::vector<std::string>& args);

  /**
   * @brief 析构函数，调用插件的deinit方法（若存在的话），使用dlclose关闭插件库
   * 
   */
  ~Plugin();

public:
  /**
   * @brief 返回插件名
   * 
   * @return std::string_view 
   */
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
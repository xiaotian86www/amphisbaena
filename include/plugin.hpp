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
 * @code {.cpp}
 * // 1. 需要将插件编译成动态库
 * extern "C" {
 * // 2. 定义初始化方法
 * void init(const char* const* args) {}
 * // 3. 定义析构方法（可选）
 * void deinit() {}
 * }
 * @endcode
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
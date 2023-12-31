/**
 * @file builder.hpp
 * @author duchang (xiaotian86www@163.com)
 * @brief 消息建造者类
 * @version 0.1
 * @date 2023-08-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include "message.hpp"
#include "schedule.hpp"

namespace amphisbaena {

class Environment;

class MessageBuilder
{
public:
  virtual ~MessageBuilder() = default;

public:
  virtual MessagePtr create(Environment& env, MessagePtr request) = 0;

  virtual std::string_view name() const = 0;

public:
  /**
   * @brief 注册
   * 
   * @param builder 
   * 
   * 要求线程安全
   */
  static void registe(std::shared_ptr<MessageBuilder> builder);

  /**
   * @brief 取消全部注册
   * 
   * 要求线程安全
   */
  static void unregiste();

  /**
   * @brief 取消注册
   * 
   * @param builder 
   *
   * 要求线程安全
   */
  static void unregiste(std::shared_ptr<MessageBuilder> builder);

  static MessagePtr create(Environment& env,
                           std::string_view pattern,
                           MessagePtr request);

private:
  static std::shared_ptr<
    std::map<std::string, std::shared_ptr<MessageBuilder>, std::less<>>>
    builders_;
};

}
/**
 * @file server.hpp
 * @author duchang (xiaotian86www@163.com)
 * @brief 服务类
 * @version 0.1
 * @date 2023-08-23
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <memory>
#include <string_view>

#include "connection.hpp"
#include "schedule.hpp"

namespace amphisbaena {

/**
 * @brief 服务端
 *
 */
class Server : public std::enable_shared_from_this<Server>
{
public:
  /**
   * @brief 构造函数
   *
   * @param message_handler 消息回调句柄
   */
  Server(Connection::MessageHandler& message_handler)
    : message_handler_(message_handler)
  {
  }

  virtual ~Server() = default;

protected:
  Connection::MessageHandler& message_handler_;
};

/**
 * @brief 服务端工厂
 *
 */
class ServerFactory
{
public:
  virtual ~ServerFactory() = default;

public:
  /**
   * @brief 创建
   *
   * @param message_handler 消息回调句柄
   * @return std::unique_ptr< @link Server @endlink > 服务端
   */
  virtual std::unique_ptr<Server> create(
    Connection::MessageHandler& message_handler) = 0;
};

}
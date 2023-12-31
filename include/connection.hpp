/**
 * @file connection.hpp
 * @author duchang (xiaotian86www@163.com)
 * @brief 连接类
 * @version 0.1
 * @date 2023-08-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */
 
#pragma once

#include <memory>
#include <string_view>

#include "schedule.hpp"

namespace amphisbaena {

class Connection;
typedef std::shared_ptr<Connection> ConnectionPtr;

/**
 * @brief 连接
 *
 */
class Connection : public std::enable_shared_from_this<Connection>
{
public:
  /**
   * @brief 消息回调句柄
   *
   */
  class MessageHandler
  {
  public:
    virtual ~MessageHandler() = default;

  public:
    /**
     * @brief 消息回调
     *
     * @param sch 协程调度器
     * @param co 协程
     * @param conn 连接
     * @param data 数据
     */
    virtual void on_recv(ScheduleRef sch,
                         CoroutineRef co,
                         ConnectionPtr conn,
                         std::string_view data) = 0;
  };

public:
  /**
   * @brief 构造函数
   *
   * @param sch 协程调度器
   * @param co 协程
   * @param message_handler 消息回调句柄
   */
  Connection(ScheduleRef sch, CoroutineRef co, MessageHandler& message_handler)
    : sch_(sch)
    , co_(co)
    , message_handler_(message_handler)
  {
  }

  virtual ~Connection() = default;

public:
  /**
   * @brief 发送数据
   *
   * @param data 数据
   */
  virtual void send(std::string_view data) = 0;

  /**
   * @brief 接受数据，并触发回调
   *
   * @return true 成功
   * @return false 失败，会触发连接关闭
   */
  virtual bool recv() = 0;

  /**
   * @brief 关闭连接
   *
   */
  virtual void close() = 0;

  /**
   * @brief 触发回调
   *
   * @param data 数据
   */
  void do_recv(std::string_view data)
  {
    message_handler_.on_recv(sch_, co_, shared_from_this(), data);
  }

protected:
  ScheduleRef sch_;
  CoroutineRef co_;

private:
  MessageHandler& message_handler_;
};

/**
 * @brief 连接引用
 *
 */
class ConnectionRef
{
public:
  ConnectionRef() {}

  ConnectionRef(std::shared_ptr<Connection> connection)
    : connection_(connection)
  {
  }

  ConnectionRef(std::weak_ptr<Connection> connection)
    : connection_(connection)
  {
  }

public:
  /**
   * @brief 发送数据
   *
   * @param data 数据
   */
  void send(std::string_view data)
  {
    if (auto connection = connection_.lock()) {
      connection->send(data);
    }
  }

private:
  std::weak_ptr<Connection> connection_;
};

}
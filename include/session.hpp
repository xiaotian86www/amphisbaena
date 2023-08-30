/**
 * @file session.hpp
 * @author duchang (xiaotian86www@163.com)
 * @brief 会话类
 * @version 0.1
 * @date 2023-08-23
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <memory>

#include "message.hpp"
#include "schedule.hpp"

namespace amphisbaena {
class Session;
typedef std::shared_ptr<Session> SessionPtr;

class Session : public std::enable_shared_from_this<Session>
{
public:
  class MessageHandler
  {
  public:
    virtual ~MessageHandler() = default;

  public:
    virtual void on_recv(ScheduleRef sch,
                         CoroutineRef co,
                         SessionPtr session,
                         MessagePtr message) = 0;
  };

public:
  Session(ScheduleRef sch, CoroutineRef co, MessageHandler& message_handler)
    : sch_(sch)
    , co_(co)
    , message_handler_(message_handler)
  {
  }

  virtual ~Session() = default;

public:
  virtual void send(MessagePtr data) = 0;

  void do_recv(MessagePtr data)
  {
    message_handler_.on_recv(sch_, co_, shared_from_this(), data);
  }

public:
  std::shared_ptr<Session> up;

protected:
  ScheduleRef sch_;
  CoroutineRef co_;

private:
  MessageHandler& message_handler_;
};

}
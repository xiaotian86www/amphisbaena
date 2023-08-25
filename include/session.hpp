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

#include "message.hpp"
#include "schedule.hpp"
#include <memory>

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
  virtual ~Session() = default;

public:
  virtual void send(MessagePtr data) = 0;

public:
  std::shared_ptr<Session> up;
};

}
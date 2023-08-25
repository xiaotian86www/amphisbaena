/**
 * @file environment.hpp
 * @author duchang (xiaotian86www@163.com)
 * @brief 上下文类
 * @version 0.1
 * @date 2023-08-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include "builder.hpp"
#include "message.hpp"
#include "schedule.hpp"
#include "session.hpp"

namespace amphisbaena {
class Environment
{
public:
  ScheduleRef sch;
  CoroutineRef co;
  SessionPtr up;
  SessionPtr down;
};
} // namespace amphisbaena

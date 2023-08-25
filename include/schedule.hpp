/**
 * @file schedule.hpp
 * @author duchang (xiaotian86www@163.com)
 * @brief 协程类
 * @version 0.1
 * @date 2023-08-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/coroutine2/coroutine.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <unordered_set>

namespace amphisbaena {

class Coroutine;
class CoroutineRef;
class Schedule;
class ScheduleRef;

typedef std::function<void(ScheduleRef, CoroutineRef)> task;

/**
 * @brief 协程
 * 
 */
class Coroutine : public std::enable_shared_from_this<Coroutine>
{
public:
  Coroutine(boost::asio::io_service& ios, std::weak_ptr<Schedule> sch, task fn);

  virtual ~Coroutine() = default;

public:
  /**
   * @brief 让渡时间片，直到调用 Schedule::resume(CoroutineRef)
   * 
   */
  void yield();

  /**
   * @brief 让渡时间片，直到 milli 毫秒后，或者调用 Schedule::resume(CoroutineRef)
   * 
   * @param milli 等待时长
   */
  void yield_for(int milli);

  /**
   * @brief 恢复
   * 
   * @return true 
   * @return false 
   */
  bool resume();

private:
  void do_yield();

private:
  std::weak_ptr<Schedule> sch_;
  boost::asio::steady_timer timer_;
  boost::coroutines2::coroutine<void>::push_type ps_;
  boost::coroutines2::coroutine<void>::pull_type* pl_ = nullptr;
};

/**
 * @brief 协程引用
 * 
 */
class CoroutineRef
{
  friend class Schedule;

public:
  CoroutineRef() {}

  CoroutineRef(std::weak_ptr<Coroutine> co)
    : co_(co)
  {
  }

  CoroutineRef(std::shared_ptr<Coroutine> co)
    : co_(co)
  {
  }

public:
  void yield();

  void yield_for(int milli);

private:
  std::weak_ptr<Coroutine> co_;
};

/**
 * @brief 调度器
 * 
 */
class Schedule : public std::enable_shared_from_this<Schedule>
{
public:
  Schedule(boost::asio::io_service& ios);
  ~Schedule() = default;
  Schedule(const Schedule&) = delete;
  Schedule& operator=(const Schedule&) = delete;

public:
  /**
   * @brief 产生协程
   * 
   * @param func 
   */
  void spawn(task func);

  /**
   * @brief 恢复协程
   * 
   * @param co 协程
   */
  void resume(CoroutineRef co);

private:
  boost::asio::io_service& ios_;
  std::mutex cos_mtx_;
  std::unordered_set<std::shared_ptr<Coroutine>> cos_;
};

/**
 * @brief 调度器引用
 * 
 */
class ScheduleRef
{
public:
  ScheduleRef() {}

  ScheduleRef(std::weak_ptr<Schedule> sch)
    : sch_(sch)
  {
  }

public:
  void resume(CoroutineRef co);

private:
  std::weak_ptr<Schedule> sch_;
};

} // namespace amphisbaena

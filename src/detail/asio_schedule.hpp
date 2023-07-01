#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/coroutine2/coroutine.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "schedule.hpp"

using namespace boost::coroutines2;

namespace translator {

class AsioCoroutine : public Coroutine
{
public:
  AsioCoroutine(std::shared_ptr<Schedule::Impl> sch);

public:
  void set_func(task&& fn);

  void yield() override;

  void yield_for(int milli) override;

  void resume() override;

private:
  std::weak_ptr<Schedule::Impl> sch_;
  boost::asio::steady_timer timer_;
  coroutine<void>::push_type ps_;
  coroutine<void>::pull_type* pl_;
};

class Schedule::Impl : public std::enable_shared_from_this<Schedule::Impl>
{
public:
  Impl();

public:
  void run();

  void stop();

  void post(task&& fn);

  // void yield();

  // void yield_for(int milli);

  // void resume(std::weak_ptr<Coroutine> co);

public:
  // std::weak_ptr<Coroutine> this_co()
  // {
  //   assert(running_co_);
  //   return running_co_;
  // }

  boost::asio::io_service& io_service() { return ios_; }

// private:
//   void do_resume(std::shared_ptr<Coroutine> co);

private:
  boost::asio::io_service ios_;
  // std::unordered_set<std::shared_ptr<Coroutine>> cos_;
  // std::shared_ptr<Coroutine> running_co_;
};

}
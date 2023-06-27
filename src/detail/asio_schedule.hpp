#include <array>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
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
using namespace boost::asio::local;

namespace translator {

struct AsioCoroutine : Schedule::Coroutine
{
  template<typename Fn>
  AsioCoroutine(boost::asio::io_service& ios, Fn&& fn)
    : timer(ios)
    , resume(
        [this, fn = std::move(fn)](coroutine<void>::pull_type& pl) mutable {
          yield = &pl;
          state = Schedule::CoroutineState::RUNNING;
          fn();
          state = Schedule::CoroutineState::DEAD;
        })
    , yield(nullptr)
  {
  }

  boost::asio::steady_timer timer;
  coroutine<void>::push_type resume;
  coroutine<void>::pull_type* yield;
};

class Schedule::Impl : public std::enable_shared_from_this<Schedule::Impl>
{
  // public:
  //   struct awake_context : public std::enable_shared_from_this<awake_context>
  //   {
  //     template<typename Fn>
  //     explicit awake_context(std::weak_ptr<Schedule2> sch, Fn&& fn)
  //       : sch_(sch)
  //       , ps_(std::forward<Fn>(fn))
  //     {
  //     }

  //     void operator()(boost::system::error_code&& ec)
  //     {
  //       auto sch = sch_.lock();
  //       if (sch) {
  //         assert(!sch->current_awake_);
  //         sch->current_awake_ = shared_from_this();
  //         ps_(std::move(ec));
  //       }
  //     }

  //     void operator()(const boost::system::error_code& ec)
  //     {
  //       auto sch = sch_.lock();
  //       if (sch) {
  //         assert(!sch->current_awake_);
  //         sch->current_awake_ = shared_from_this();
  //         ps_(ec);
  //         sch->current_awake_.reset();
  //       }
  //     }

  //     bool invalid() { return !ps_; }

  //     coroutine<boost::system::error_code>::push_type ps_;
  //     std::weak_ptr<Schedule2> sch_;
  //   };

  //   struct await_context
  //   {
  //     await_context(std::weak_ptr<Schedule2> sch,
  //                   coroutine<boost::system::error_code>::pull_type& pl)
  //       : sch_(sch)
  //       , pl_(pl)
  //     {
  //     }

  //     boost::system::error_code operator()() { return pl_().get(); }

  //     std::weak_ptr<awake_context> awake()
  //     {
  //       auto sch = sch_.lock();
  //       if (sch) {
  //         assert(sch->current_awake_);
  //         return sch->current_awake_;
  //       }

  //       return std::weak_ptr<awake_context>();
  //     }

  //     coroutine<boost::system::error_code>::pull_type& pl_;
  //     std::weak_ptr<Schedule2> sch_;
  //   };

public:
  Impl();

public:
  void run();

  void stop();

  void post(task&& fn);

  void yield();

  void yield_for(int milli);

  void resume(std::weak_ptr<Schedule::Coroutine> co);

public:
  std::weak_ptr<Coroutine> this_co()
  {
    assert(running_co_);
    return running_co_;
  }

private:
  void do_resume(std::shared_ptr<AsioCoroutine> co);

public:
  // void do_accept(await_context await);

  // void do_read(std::shared_ptr<stream_protocol::socket> sock,
  //              await_context await);

private:
  boost::asio::io_service ios_;
  // stream_protocol::acceptor acceptor_;
  // std::array<char, 1024> recv_buffer_;
  std::unordered_set<std::shared_ptr<AsioCoroutine>> cos_;
  std::shared_ptr<AsioCoroutine> running_co_;
};

}
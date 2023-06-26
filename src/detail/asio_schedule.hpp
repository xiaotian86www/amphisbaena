#include <array>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/coroutine2/coroutine.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "schedule.hpp"

using namespace boost::coroutines2;
using namespace boost::asio::local;

namespace translator {

struct Coroutine2 : public std::enable_shared_from_this<Coroutine2>
{
  template<typename Fn>
  Coroutine2(Fn&& fn)
    : yield(nullptr)
    , resume([this, fn = std::forward<Fn>(fn)](
               coroutine<boost::system::error_code>::pull_type& pl) {
      yield = &pl;
      fn(shared_from_this());
    })
  {
  }

  coroutine<boost::system::error_code>::pull_type* yield;
  coroutine<boost::system::error_code>::push_type resume;
};

class Schedule2 : public std::enable_shared_from_this<Schedule2>
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
  Schedule2();

public:
  template<typename Fn>
  void post(Fn&& fn)
  {
    auto co = std::make_shared<Coroutine2>(std::forward<Fn>(fn));
    cos_.insert(co);
    ios_.post([co] {
      co->resume(
        boost::system::errc::make_error_code(boost::system::errc::success));
    });
  }

  void yield();

public:
  // void do_accept(await_context await);

  // void do_read(std::shared_ptr<stream_protocol::socket> sock,
  //              await_context await);

private:
  boost::asio::io_service ios_;
  // stream_protocol::acceptor acceptor_;
  // std::array<char, 1024> recv_buffer_;
  std::unordered_set<std::shared_ptr<Coroutine2>> cos_;
  std::shared_ptr<Coroutine2> running_co_;
};

}
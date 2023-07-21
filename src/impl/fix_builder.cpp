#include <boost/scope_exit.hpp>
#include <memory>
#include <mutex>
#include <quickfix/FixValues.h>

#include "environment.hpp"
#include "fix_builder.hpp"
#include "future.hpp"
#include "message.hpp"

namespace translator {
FixMessageBuilder::FixMessageBuilder(std::unique_ptr<Service> service, int timeout_milli)
  : service_(std::move(service))
  , timeout_milli_(timeout_milli)
{
  service_->message_handler = this;
  service_->start();
}

FixMessageBuilder::~FixMessageBuilder()
{
  service_->stop();
}

MessagePtr
FixMessageBuilder::operator()(Environment& env, MessagePtr request)
{
  if (!env.up) {
    env.up = service_->create(request);
    if (!env.up) {
      // TODO 查找不到会话
      return MessagePtr();
    }
  }


  // auto cl_ord_id = request->get_body().get_string("ClOrdID");

  Promise<MessagePtr> pms(env.sch, env.co);
  auto ftr = pms.future();

  {
    std::lock_guard<std::mutex> lg(pmss_mtx_);
    pmss_.insert_or_assign(env.up, std::move(pms));
  }
  BOOST_SCOPE_EXIT(this_, &env)
  {
    std::lock_guard<std::mutex> lg(this_->pmss_mtx_);
    this_->pmss_.erase(env.up);
  }
  BOOST_SCOPE_EXIT_END

  env.up->send(request);

  for (;;) {
    auto response = ftr.get_for(timeout_milli_, MessagePtr());
    if (!response) {
      // TODO 超时
      return response;
    }

    if (request->get_head().get_string("MsgType") ==
        FIX::MsgType_NewOrderSingle) {
      if (response->get_head().get_string("MsgType") ==
          FIX::MsgType_ExecutionReport)
        return response;
    }
  }
}

void
FixMessageBuilder::on_recv(ScheduleRef sch, CoroutineRef co, SessionPtr session, MessagePtr response)
{
  // auto cl_ord_id = response->get_body().get_string("ClOrdID");

  std::lock_guard<std::mutex> lg(pmss_mtx_);
  if (auto iter = pmss_.find(session); iter != pmss_.end()) {
    iter->second.set(std::move(response));
  }
}

}
#include <boost/scope_exit.hpp>
#include <memory>
#include <mutex>
#include <quickfix/FixValues.h>
#include <stdexcept>

#include "builder.hpp"
#include "environment.hpp"
#include "fix_builder.hpp"
#include "fix_client.hpp"
#include "future.hpp"
#include "log.hpp"
#include "message.hpp"

namespace amphisbaena {
FixBuilder::FixBuilder(ClientFactory& client_factory, int timeout_milli)
  : client_(client_factory.create(this))
  , timeout_milli_(timeout_milli)
{
  LOG_INFO("FixBuilder create");
}

FixBuilder::~FixBuilder()
{
  LOG_INFO("FixBuilder destroy");
}

MessagePtr
FixBuilder::create(Environment& env, MessagePtr request)
{
  if (!env.up) {
    env.up = client_->create(request);
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
      throw TimeoutException();
    }

    if (auto request_head = request->get_head();
        request_head->get_string("MsgType") == FIX::MsgType_NewOrderSingle) {
      if (auto response_head = response->get_head();
          response_head->get_string("MsgType") == FIX::MsgType_ExecutionReport)
        return response;
    }
  }
}

std::string_view
FixBuilder::name() const
{
  return "fix";
}

void
FixBuilder::on_recv(ScheduleRef /* sch */,
                    CoroutineRef /* co */,
                    SessionPtr session,
                    MessagePtr response)
{
  std::lock_guard<std::mutex> lg(pmss_mtx_);
  if (auto iter = pmss_.find(session); iter != pmss_.end()) {
    iter->second.set(std::move(response));
  }
}

}
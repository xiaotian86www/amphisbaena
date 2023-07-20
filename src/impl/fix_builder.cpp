#include <boost/scope_exit.hpp>
#include <memory>
#include <mutex>
#include <quickfix/FixValues.h>

#include "environment.hpp"
#include "fix_builder.hpp"
#include "future.hpp"

namespace translator {
FixMessageBuilder::FixMessageBuilder(std::unique_ptr<Service> service, int timeout_milli)
  : service_(std::move(service))
  , timeout_milli_(timeout_milli)
{
  service_->handler = this;
  service_->start();
}

FixMessageBuilder::~FixMessageBuilder()
{
  service_->stop();
}

MessagePtr
FixMessageBuilder::operator()(Environment& env, MessagePtr request)
{
  auto cl_ord_id = request->get_body().get_string("ClOrdID");

  Promise<MessagePtr> pms(env.sch, env.co);
  auto ftr = pms.future();

  {
    std::lock_guard<std::mutex> lg(pmss_mtx_);
    pmss_.insert_or_assign(std::string(cl_ord_id), std::move(pms));
  }
  BOOST_SCOPE_EXIT(this_, cl_ord_id)
  {
    std::lock_guard<std::mutex> lg(this_->pmss_mtx_);
    this_->pmss_.erase(std::string(cl_ord_id));
  }
  BOOST_SCOPE_EXIT_END

  service_->send(request);

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
FixMessageBuilder::on_message(MessagePtr response)
{
  auto cl_ord_id = response->get_body().get_string("ClOrdID");

  std::lock_guard<std::mutex> lg(pmss_mtx_);
  if (auto iter = pmss_.find(cl_ord_id); iter != pmss_.end()) {
    iter->second.set(std::move(response));
  }
}

}
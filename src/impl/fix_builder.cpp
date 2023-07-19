#include <boost/scope_exit.hpp>
#include <memory>
#include <mutex>
#include <quickfix/FixValues.h>

#include "environment.hpp"
#include "fix_builder.hpp"
#include "future.hpp"

namespace translator {
FixMessageBuilder::FixMessageBuilder(std::istream& is)
  : FixClient(is)
{
  start();
}

MessagePtr
FixMessageBuilder::operator()(Environment& env, MessagePtr request)
{
  send(std::static_pointer_cast<FixMessage>(request));
  auto cl_ord_id = request->get_body().get_string("ClOrdID");

  Promise<MessagePtr> pms(env.sch, env.co);
  auto ftr = pms.future();

  {
    std::lock_guard<std::mutex> lg(pmss_mtx_);
    pmss_.insert_or_assign(std::string(cl_ord_id), std::move(pms));
  }
  BOOST_SCOPE_EXIT(cl_ord_id, &pmss_)
  {
    pmss_.erase(std::string(cl_ord_id));
  }
  BOOST_SCOPE_EXIT_END

  for (;;) {
    auto response = ftr.get_for(5000, MessagePtr());
    if (!response) {
      // TODO 超时
      return response;
    }

    if (request->get_body().get_string("MsgType") ==
        FIX::MsgType_NewOrderSingle) {
      if (response->get_body().get_string("MsgType") ==
          FIX::MsgType_ExecutionReport)
        return response;
    }
  }
}

}
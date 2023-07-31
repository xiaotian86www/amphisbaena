#pragma once

#include <map>

#include "builder.hpp"
#include "client.hpp"
#include "future.hpp"
#include "message.hpp"

namespace amphisbaena {

class FixBuilder
  : public Client::MessageHandler
  , public MessageBuilder
{
public:
  explicit FixBuilder(ClientFactory& client_factory,
                             int timeout_milli = 1000);

  ~FixBuilder() override;

public:
  MessagePtr create(Environment& env, MessagePtr request) override;

  std::string_view name() const override;

public:
  void on_recv(ScheduleRef sch,
               CoroutineRef co,
               SessionPtr session,
               MessagePtr response) override;

private:
  std::map<SessionPtr, Promise<MessagePtr>> pmss_;
  std::mutex pmss_mtx_;
  std::unique_ptr<Client> service_;
  int timeout_milli_;
};
}
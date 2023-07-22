#pragma once

#include <map>

#include "builder.hpp"
#include "future.hpp"
#include "message.hpp"
#include "client.hpp"

namespace translator {

class FixMessageBuilder : public Client::MessageHandler
{
public:
  explicit FixMessageBuilder(std::unique_ptr<Client> service,
                             int timeout_milli = 1000);

  ~FixMessageBuilder() override;

public:
  MessagePtr operator()(Environment& env, MessagePtr request);

public:
  void on_recv(ScheduleRef sch, CoroutineRef co, SessionPtr session, MessagePtr response) override;

private:
  std::map<SessionPtr, Promise<MessagePtr>> pmss_;
  std::mutex pmss_mtx_;
  std::unique_ptr<Client> service_;
  int timeout_milli_;
};
}
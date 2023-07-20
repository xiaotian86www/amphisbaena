#pragma once

#include <map>

#include "builder.hpp"
#include "future.hpp"
#include "message.hpp"
#include "service.hpp"

namespace translator {

class FixMessageBuilder : public Service::MessageHandler
{
public:
  FixMessageBuilder(std::unique_ptr<Service> service);

  ~FixMessageBuilder() override;

public:
  MessagePtr operator()(Environment& env, MessagePtr request);

private:
  void on_message(MessagePtr response) override;

private:
  std::map<std::string, Promise<MessagePtr>, std::less<>> pmss_;
  std::mutex pmss_mtx_;
  std::unique_ptr<Service> service_;
};
}
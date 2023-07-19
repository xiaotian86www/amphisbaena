#pragma once

#include "builder.hpp"
#include "fix_client.hpp"
#include "fix_message.hpp"
#include "future.hpp"
#include "message.hpp"

namespace translator {

class FixMessageBuilder : private FixClient
{
public:
  FixMessageBuilder(std::istream& is);

public:
  MessagePtr operator()(Environment& env, MessagePtr request);

private:
  void on_data(FixMessagePtr message) override;

private:
  std::map<std::string, Promise<MessagePtr>, std::less<>> pmss_;
  std::mutex pmss_mtx_;
};
}
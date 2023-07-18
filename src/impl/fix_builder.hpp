#pragma once

#include "builder.hpp"
#include "fix_client.hpp"
#include "fix_message.hpp"
#include "message.hpp"

namespace translator {

class FixMessageBuilder
{
public:
  FixMessageBuilder(std::istream& is);

public:
  MessagePtr operator()(Environment& env) const;

private:
  FixClient client_;
};
}
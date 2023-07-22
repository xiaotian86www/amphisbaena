#pragma once

#include "message.hpp"

namespace translator {
class Session
{
public:
  virtual ~Session() = default;

public:
  virtual void send(MessagePtr data) = 0;
};

typedef std::shared_ptr<Session> SessionPtr;
}
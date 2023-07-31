#pragma once

#include "message.hpp"

namespace amphisbaena {
class Session
{
public:
  virtual ~Session() = default;

public:
  virtual void send(MessagePtr data) = 0;
};

typedef std::shared_ptr<Session> SessionPtr;
}
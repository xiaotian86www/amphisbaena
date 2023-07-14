#pragma once

namespace translator {
class Service
{
public:
  virtual ~Service() = default;

public:
  virtual void start() = 0;

  virtual void stop() = 0;
};
}
#pragma once

#include <memory>
#include <string_view>

#include "schedule.hpp"

namespace translator {
class Socket : public std::enable_shared_from_this<Socket>
{
public:
  virtual ~Socket() = default;

public:
  virtual void send(ScheduleRef sch,
                    CoroutineRef co,
                    std::string_view data) = 0;
};

}
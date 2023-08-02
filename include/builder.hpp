#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include "message.hpp"
#include "schedule.hpp"

namespace amphisbaena {

class Environment;

class MessageBuilder
{
public:
  virtual ~MessageBuilder() = default;

public:
  virtual MessagePtr create(Environment& env, MessagePtr request) = 0;

  virtual std::string_view name() const = 0;

public:
  static void registe(std::shared_ptr<MessageBuilder> builder);

  static void unregiste();

  static void unregiste(std::shared_ptr<MessageBuilder> builder);

  static MessagePtr create(Environment& env,
                           std::string_view name,
                           MessagePtr request);

private:
  static std::shared_ptr<
    std::map<std::string, std::shared_ptr<MessageBuilder>, std::less<>>>
    builders_;
};

}
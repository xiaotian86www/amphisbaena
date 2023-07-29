#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include "message.hpp"
#include "schedule.hpp"

namespace translator {

class Environment;

class MessageBuilder
{
public:
  virtual ~MessageBuilder() = default;

public:
  virtual MessagePtr create(Environment& env, MessagePtr request) = 0;

  virtual std::string_view name() const = 0;

public:
  static void registe(
    std::map<std::string_view, std::shared_ptr<MessageBuilder>> builders);

  static void unregiste();

  static void unregiste(const std::vector<std::string_view>& names);

  static MessagePtr create(Environment& env,
                           std::string_view name,
                           MessagePtr request);

private:
  static std::shared_ptr<
    std::map<std::string, std::shared_ptr<MessageBuilder>, std::less<>>>
    builders_;
};

class Plugin
{
public:
  static void load(const std::vector<std::filesystem::path>& paths);
};
}
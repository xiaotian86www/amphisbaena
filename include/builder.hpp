#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string_view>

#include "message.hpp"
#include "schedule.hpp"

namespace translator {

class Environment;

class MessageBuilder
{
public:
  using ctor_prototype = MessagePtr(Environment&, MessagePtr);
  using ctor_function = std::function<ctor_prototype>;

public:
  static void registe(std::map<std::string_view, ctor_function> ctors);

  static void unregiste();

  static MessagePtr create(Environment& env,
                           std::string_view name,
                           MessagePtr request);

private:
  static std::shared_ptr<std::map<std::string, ctor_function, std::less<>>>
    ctors_;
};

class Plugin
{
public:
  static void load(const std::vector<std::filesystem::path>& paths);
};
}
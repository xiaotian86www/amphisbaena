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

class Plugin
{
public:
  Plugin(const std::filesystem::path& path);

  Plugin(const std::filesystem::path& path,
         const std::vector<std::string>& args);

  ~Plugin();

private:
  void init(const std::vector<const char*>& args);

  void deinit();

private:
  std::filesystem::path path_;
  void* handle_;

// public:
//   static void load(
//     const std::map<std::filesystem::path, std::vector<std::string>>& infos);

//   static void load(const std::filesystem::path& path);

//   static void load(const std::filesystem::path& path,
//                    const std::vector<std::string>& args);
};
}
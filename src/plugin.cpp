
#include <dlfcn.h>
#include <filesystem>

#include "exception.hpp"
#include "log.hpp"
#include "plugin.hpp"

namespace amphisbaena {
Plugin::Plugin(std::string_view name,
               const std::filesystem::path& path,
               const std::vector<std::string>& args)
  : name_(name)
  , path_(path)
{
  LOG_INFO("Load plugin path: {}", path_.string());
  handle_ = dlopen(path_.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (!handle_)
    throw CouldnotLoadException(path_.string(), dlerror());

  try {
    std::vector<const char*> args_;
    args_.push_back(path_.c_str());
    for (const auto& arg : args) {
      args_.push_back(arg.c_str());
    }

    init(args_);
  } catch (...) {
    dlclose(handle_);
    throw;
  }
}

Plugin::~Plugin()
{
  LOG_INFO("Unload plugin path: {}", path_.string());
  deinit();

  dlclose(handle_);
}

void
Plugin::init(const std::vector<const char*>& args)
{
  assert(!args.empty());

  void (*init)(int, const char* const*) = nullptr;
  *(void**)(&init) = dlsym(handle_, "init");
  if (!init)
    throw CouldnotLoadException(args[0], dlerror());

  init(args.size(), args.data());
}

void
Plugin::deinit()
{
  void (*deinit)() = nullptr;
  *(void**)(&deinit) = dlsym(handle_, "deinit");
  if (deinit)
    deinit();
}

}
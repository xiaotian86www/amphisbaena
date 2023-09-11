#pragma once

#include <atomic>

namespace amphisbaena {
class Loader
{
public:
  Loader() {}
  virtual ~Loader() {}

public:
  virtual void init(int argc, const char* const* argv)
  {
    if (count_++ == 0)
      do_init(argc, argv);
  }

  virtual void deinit()
  {
    if (--count_ == 0)
      do_deinit();
  }

protected:
  virtual void do_init(int argc, const char* const* argv) = 0;

  virtual void do_deinit() = 0;

private:
  std::atomic<int> count_;
};
}

#define AMP_REGISTE_PLUGIN_LOADER(cls)                                                   \
  static cls g_loader_;                                                        \
  extern "C"                                                                   \
  {                                                                            \
    void init(int argc, const char* const* argv)                               \
    {                                                                          \
      g_loader_.init(argc, argv);                                              \
    }                                                                          \
                                                                               \
    void deinit()                                                              \
    {                                                                          \
      g_loader_.deinit();                                                      \
    }                                                                          \
  }

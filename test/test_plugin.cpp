#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "environment.hpp"
#include "plugin.hpp"

class Plugin : public testing::Test
{
public:
  Plugin() {}
  ~Plugin() { amphisbaena::MessageBuilder::unregiste(); }
};

TEST_F(Plugin, load)
{
  amphisbaena::Environment env;

  amphisbaena::Plugin plugin1("plugin1", "mock/plugin/libplugin1.so", {});
  {
    auto message = amphisbaena::MessageBuilder::create(
      env, "Message", amphisbaena::MessagePtr());

    EXPECT_EQ(message->get_body()->get_int("Field"), 1);
  }

  amphisbaena::Plugin plugin13("plugin13", "mock/plugin/libplugin11.so", {});
  {
    auto message = amphisbaena::MessageBuilder::create(
      env, "Message", amphisbaena::MessagePtr());

    EXPECT_EQ(message->get_body()->get_int("Field"), 2);
  }
}

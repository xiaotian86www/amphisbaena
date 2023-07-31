#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "builder.hpp"
#include "environment.hpp"

class Plugin : public testing::Test
{
public:
  Plugin() {}
  ~Plugin() { amphisbaena::MessageBuilder::unregiste(); }
};

TEST_F(Plugin, load)
{
  amphisbaena::Environment env;

  amphisbaena::Plugin::load("mock/builder/libbuilder1.so", {});
  {
    auto message = amphisbaena::MessageBuilder::create(
      env, "Message", amphisbaena::MessagePtr());

    EXPECT_EQ(message->get_body()->get_int("Field"), 1);
  }

  amphisbaena::Plugin::load("mock/builder/libbuilder13.so", {});
  {
    auto message = amphisbaena::MessageBuilder::create(
      env, "Message", amphisbaena::MessagePtr());

    EXPECT_EQ(message->get_body()->get_int("Field"), 2);
  }
}

TEST_F(Plugin, load_fail)
{
  amphisbaena::Environment env;

  EXPECT_THROW(amphisbaena::Plugin::load("mock/builder/libbuilder11.so", {}),
               amphisbaena::CouldnotLoadException);
  EXPECT_THROW(amphisbaena::Plugin::load("mock/builder/libbuilder12.so", {}),
               std::invalid_argument);
}
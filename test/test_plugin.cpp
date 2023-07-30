#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "builder.hpp"
#include "environment.hpp"

class Plugin : public testing::Test
{
public:
  Plugin() {}
  ~Plugin() { translator::MessageBuilder::unregiste(); }
};

TEST_F(Plugin, load)
{
  translator::Environment env;

  translator::Plugin::load("mock/builder/libbuilder1.so", {});
  {
    auto message = translator::MessageBuilder::create(
      env, "Message", translator::MessagePtr());

    EXPECT_EQ(message->get_body()->get_int("Field"), 1);
  }

  translator::Plugin::load("mock/builder/libbuilder13.so", {});
  {
    auto message = translator::MessageBuilder::create(
      env, "Message", translator::MessagePtr());

    EXPECT_EQ(message->get_body()->get_int("Field"), 2);
  }
}

TEST_F(Plugin, load_fail)
{
  translator::Environment env;

  EXPECT_THROW(translator::Plugin::load("mock/builder/libbuilder11.so", {}),
               translator::CouldnotLoadException);
  EXPECT_THROW(translator::Plugin::load("mock/builder/libbuilder12.so", {}),
               std::invalid_argument);
}
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "application.hpp"

class Application : public testing::Test
{
public:
  void TearDown() override
  {
    amphisbaena::Application::get_instance().unload();
  }
};

TEST_F(Application, load)
{
  EXPECT_TRUE(amphisbaena::Application::get_instance().load(
    "a", "mock/plugin/libplugin1.so", {}));
  EXPECT_TRUE(!amphisbaena::Application::get_instance().load(
    "a", "mock/plugin/libplugin1.so", {}));
//   EXPECT_TRUE(amphisbaena::Application::get_instance().load(
//     "b", "mock/plugin/libplugin1.so", {}));
}

TEST_F(Application, reload)
{
  EXPECT_TRUE(amphisbaena::Application::get_instance().load(
    "a", "mock/plugin/libplugin1.so", {}));
  EXPECT_TRUE(amphisbaena::Application::get_instance().reload(
    "a", "mock/plugin/libplugin1.so", {}));
  EXPECT_TRUE(!amphisbaena::Application::get_instance().reload(
    "b", "mock/plugin/libplugin1.so", {}));
}

TEST_F(Application, unload) {}
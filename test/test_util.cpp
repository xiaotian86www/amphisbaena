
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "util.hpp"

TEST(Util, split)
{
  std::string_view str = "/1/2/3/4/5";

  auto result = amphisbaena::util::split(str, "/");

  EXPECT_EQ(result.size(), 5);
  EXPECT_EQ(result.at(0), "1");
  EXPECT_EQ(result.at(1), "2");
  EXPECT_EQ(result.at(2), "3");
  EXPECT_EQ(result.at(3), "4");
  EXPECT_EQ(result.at(4), "5");
}
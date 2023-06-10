#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "context.hpp"

#include "mock/mock_object.hpp"

TEST(context, get_object)
{
  translator::Context context;

  testing::MockFunction<std::unique_ptr<translator::Object>(
    translator::Context*)>
    func;

  EXPECT_CALL(func, Call(testing::_))
    .WillOnce(testing::Return(std::make_unique<MockObject>()));

  context.set_object_func("a", func.AsStdFunction());
  context.get_object("a");
}

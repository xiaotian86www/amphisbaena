
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "context.hpp"
#include "environment.hpp"
#include "message.hpp"
#include "mock/mock_message.hpp"
#include "parser.hpp"

class Builder : public testing::Test
{
public:
  virtual void SetUp() {}

  virtual void TearDown() {}

protected:
};

TEST_F(Builder, create)
{
  auto message_builder = std::make_shared<translator::MessageBuilder>();
  translator::Context::get_instance().message_builder = message_builder;
  translator::Environment env;

  testing::MockFunction<translator::MessageBuilder::ctor_prototype>
    func;

  EXPECT_CALL(func, Call(testing::_, testing::_))
    .WillOnce(testing::Return(std::make_shared<MockMessage>()));

  message_builder->registe("a", func.AsStdFunction());
  message_builder->create(env, "a", std::make_shared<MockMessage>());
}

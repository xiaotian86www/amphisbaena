
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "context.hpp"
#include "environment.hpp"
#include "mock/mock_message.hpp"
#include "message.hpp"
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
  auto obj_factory = std::make_shared<translator::MessageBuilder>();
  translator::Context::get_instance().message_builder = obj_factory;
  translator::Environment env;

  testing::MockFunction<std::unique_ptr<translator::Message>(
    translator::Environment&)>
    func;

  EXPECT_CALL(func, Call(testing::_))
    .WillOnce(testing::Return(std::make_unique<MockMessage>()));

  obj_factory->registe("a", func.AsStdFunction());
  obj_factory->create("a", env);
}

TEST_F(Builder, get)
{
  auto obj_factory = std::make_shared<translator::MessageBuilder>();
  translator::Context::get_instance().message_builder = obj_factory;
  translator::Environment env;

  auto message = std::make_unique<MockMessage>();
  auto obj_ptr = message.get();

  env.message_pool.add("a", std::move(message));
  EXPECT_EQ(&env.message_pool.get("a", env), obj_ptr);
}

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "context.hpp"
#include "environment.hpp"
#include "mock/mock_object.hpp"
#include "object.hpp"
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
  auto obj_factory = std::make_shared<translator::ObjectBuilder>();
  translator::Context::get_instance().object_builder = obj_factory;
  translator::Environment env;

  testing::MockFunction<std::unique_ptr<translator::Object>(
    translator::Environment&)>
    func;

  EXPECT_CALL(func, Call(testing::_))
    .WillOnce(testing::Return(std::make_unique<MockObject>()));

  obj_factory->registe("a", func.AsStdFunction());
  obj_factory->create("a", env);
}

TEST_F(Builder, get)
{
  auto obj_factory = std::make_shared<translator::ObjectBuilder>();
  translator::Context::get_instance().object_builder = obj_factory;
  translator::Environment env;

  auto obj = std::make_unique<MockObject>();
  auto obj_ptr = obj.get();

  env.object_pool.add("a", std::move(obj));
  EXPECT_EQ(&env.object_pool.get("a", env), obj_ptr);
}

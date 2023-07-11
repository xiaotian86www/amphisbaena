
#include <memory>

#include "environment.hpp"
#include "mock/mock_object.hpp"
#include "object.hpp"
#include "parser.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class Object : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown() {}

protected:
};

TEST(object, create)
{
  auto obj_factory = std::make_shared<translator::ObjectFactory>();
  translator::Environment env;
  env.object_factory = obj_factory;

  testing::MockFunction<std::unique_ptr<translator::Object>(
    translator::Environment&)>
    func;

  EXPECT_CALL(func, Call(testing::_))
    .WillOnce(testing::Return(std::make_unique<MockObject>()));

  obj_factory->registe("a", func.AsStdFunction());
  obj_factory->create("a", env);
}

TEST(object, get)
{
  auto obj_factory = std::make_shared<translator::ObjectFactory>();
  translator::Environment env;
  env.object_factory = obj_factory;

  auto obj = std::make_unique<MockObject>();
  auto obj_ptr = obj.get();

  env.object_pool.add("a", std::move(obj));
  EXPECT_EQ(env.object_pool.get("a", env), obj_ptr);
}

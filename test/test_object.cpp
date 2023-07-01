
#include <memory>

#include "environment.hpp"
#include "mock/mock_object.hpp"
#include "object.hpp"
#include "server.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(object, produce_object)
{
  auto obj_factory = std::make_shared<translator::ObjectFactory>();
  // auto server_pool = std::make_shared<translator::ServerPool>();
  auto env =
    std::make_shared<translator::Environment>(obj_factory/*, server_pool*/);

  testing::MockFunction<std::unique_ptr<translator::Object>(
    translator::Environment*)>
    func;

  EXPECT_CALL(func, Call(testing::_))
    .WillOnce(testing::Return(std::make_unique<MockObject>()));

  obj_factory->registe("a", func.AsStdFunction());
  obj_factory->produce("a", env.get());
}

TEST(object, get_object)
{
  auto obj_factory = std::make_shared<translator::ObjectFactory>();
  // auto server_pool = std::make_shared<translator::ServerPool>();
  auto env =
    std::make_shared<translator::Environment>(obj_factory/*, server_pool*/);
  auto obj_pool = std::make_shared<translator::ObjectPool>(obj_factory);

  auto obj = std::make_unique<MockObject>();
  auto obj_ptr = obj.get();

  obj_pool->add("a", std::move(obj));
  EXPECT_EQ(obj_pool->get("a", env.get()), obj_ptr);
}

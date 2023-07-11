#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <memory>

#include "mock/mock_processor.hpp"
#include "processor.hpp"

class Processor : public testing::Test
{
public:
  virtual void SetUp()
  {
    EXPECT_CALL(ctor, Call())
      .WillOnce(testing::Return(std::make_shared<MockProcessor>()));

    factory.registe("method1", ctor.AsStdFunction());
  }

  virtual void TearDown() {}

protected:
  translator::ProcessorFactory factory;
  testing::MockFunction<translator::ProcessorFactory::ctor_prototype> ctor;
};

TEST_F(Processor, create)
{
  EXPECT_TRUE(factory.create("method1"));
  EXPECT_FALSE(factory.create("method2"));
}
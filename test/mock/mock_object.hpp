#pragma once

#include "gmock/gmock.h"

#include "object.hpp"

class MockObject : public translator::Object
{
public:
  MOCK_METHOD(int32_t,
              get_value,
              (std::string_view, int32_t),
              (const, override));
  MOCK_METHOD(std::string_view,
              get_value,
              (std::string_view, std::string_view),
              (const, override));
  MOCK_METHOD(void, set_value, (std::string_view, int32_t), (override));
  MOCK_METHOD(void,
              set_value,
              (std::string_view, std::string_view),
              (override));
  MOCK_METHOD(translator::Group*, get_group, (std::string_view), (override));
  MOCK_METHOD(const translator::Group*,
              get_group,
              (std::string_view),
              (const, override));
};
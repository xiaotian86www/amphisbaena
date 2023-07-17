#pragma once

#include "gmock/gmock.h"

#include "object.hpp"

class MockNode : public translator::Node
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
  MOCK_METHOD(int32_t, get_int, (std::string_view), (const, override));
  MOCK_METHOD(std::string_view,
              get_string,
              (std::string_view),
              (const, override));
  MOCK_METHOD(void, set_value, (std::string_view, int32_t), (override));
  MOCK_METHOD(void,
              set_value,
              (std::string_view, std::string_view),
              (override));
  MOCK_METHOD(translator::GroupPtr, get_group, (std::string_view), (override));
  MOCK_METHOD(const translator::GroupPtr,
              get_group,
              (std::string_view),
              (const, override));
};

class MockObject : public translator::Object
{
public:
  MOCK_METHOD(const translator::Node&, get_head, (), (const, override));
  MOCK_METHOD(translator::Node&, get_head, (), (override));
  MOCK_METHOD(const translator::Node&, get_body, (), (const, override));
  MOCK_METHOD(translator::Node&, get_body, (), (override));
  MOCK_METHOD(const translator::Node&, get_tail, (), (const, override));
  MOCK_METHOD(translator::Node&, get_tail, (), (override));
  MOCK_METHOD(std::string, to_string, (), (const, override));
  MOCK_METHOD(void, from_string, (std::string_view), (override));
  MOCK_METHOD(std::string, to_binary, (), (const, override));
  MOCK_METHOD(void, from_binary, (std::string_view), (override));
  MOCK_METHOD(void, clear, (), (override));
};

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "message.hpp"


MATCHER_P2(field_iter_string_eq, n, v, "")
{
  return arg.get_type() == amphisbaena::FieldType::kString &&
         arg.get_name() == n && arg.get_string() == v;
}

MATCHER_P2(field_iter_int_eq, n, v, "")
{
  return arg.get_type() == amphisbaena::FieldType::kInt && arg.get_name() == n &&
         arg.get_int() == v;
}

MATCHER_P2(field_iter_double_eq, n, v, "")
{
  return arg.get_type() == amphisbaena::FieldType::kDouble &&
         arg.get_name() == n && arg.get_double() == v;
}

MATCHER_P2(field_string_eq, n, v, "")
{
  return arg->get_string(n) == v;
}

MATCHER_P2(field_int_eq, n, v, "")
{
  return arg->get_int(n) == v;
}

MATCHER_P2(field_double_eq, n, v, "")
{
  return arg->get_double(n) == v;
}

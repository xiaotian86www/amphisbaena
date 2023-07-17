#pragma once

#include <gmock/gmock.h>

#include "builder.hpp"

class MockSession : public translator::Session
{
public:
  MOCK_METHOD(void,
              reply,
              (translator::ScheduleRef sch,
               translator::CoroutineRef co,
               const translator::Message& data),
              (override));
};

// class MockProcessor : public translator::Processor
// {
// public:
//   MOCK_METHOD(void,
//               handle,
//               (translator::ScheduleRef sch,
//                translator::CoroutineRef co,
//                translator::SessionRef session,
//                const translator::Message& data),
//               (override));
// };

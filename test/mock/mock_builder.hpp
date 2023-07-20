#pragma once

#include <gmock/gmock.h>

#include "message.hpp"
#include "service.hpp"

class MockSession : public translator::Session
{
public:
  MOCK_METHOD(void,
              send,
              (translator::Environment & env, translator::MessagePtr data),
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

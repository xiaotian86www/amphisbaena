#pragma once

#include "gmock/gmock.h"

#include "processor.hpp"

class MockSession : public translator::Session
{
public:
  MOCK_METHOD(void,
              reply,
              (translator::ScheduleRef sch,
               translator::CoroutineRef co,
               const translator::ResponseData& data),
              (override));
};

class MockProcessor : public translator::Processor
{
public:
  MOCK_METHOD(void,
              handle,
              (translator::ScheduleRef sch,
               translator::CoroutineRef co,
               std::shared_ptr<translator::Session> session,
               const translator::RequestData& data),
              (override));
};

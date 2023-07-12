
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include "context.hpp"
#include "detail/http_parser.hpp"
#include "mock/mock_parser.hpp"
#include "mock/mock_processor.hpp"
#include "parser.hpp"
#include "processor.hpp"
#include "schedule.hpp"

// class Parser : public testing::Test
// {
// public:
//   virtual void SetUp()
//   {
//     sch = std::make_shared<translator::Schedule>(ios);
//     parser_factory = std::make_shared<translator::HttpParserFactory>();
//     processor_factory = std::make_shared<translator::ProcessorFactory>();
//     processor = std::make_shared<MockProcessor>();
//     conn = std::make_shared<MockConnection>();

//     translator::Context::get_instance().processor_factory = processor_factory;

//     processor_factory->registe("/", [this] { return processor; });
//   }

//   virtual void TearDown() {}

// protected:
//   boost::asio::io_service ios;
//   std::thread th;
//   std::shared_ptr<translator::Schedule> sch;
//   std::shared_ptr<translator::ParserFactory> parser_factory;
//   std::shared_ptr<translator::ProcessorFactory> processor_factory;
//   std::shared_ptr<MockProcessor> processor;
//   std::shared_ptr<MockConnection> conn;
// };

// TEST_F(Parser, on_data)
// {

//   EXPECT_CALL(
//     *processor,
//     handle(
//       testing::_,
//       testing::_,
//       testing::_,
//       testing::AllOf(testing::Field(&translator::RequestData::url, "/"),
//                      testing::Field(&translator::RequestData::method, "GET"))))
//     .Times(1);

//   sch->spawn([this](translator::ScheduleRef sch, translator::CoroutineRef co) {
//     auto parser = parser_factory->create();

//     parser->on_data(sch,
//                     co,
//                     std::static_pointer_cast<translator::Connection>(conn),
//                     "GET / HTTP/1.1\r\n\r\n");
//   });

//   th = std::thread([this] { ios.run(); });

//   ios.stop();
//   th.join();
// }
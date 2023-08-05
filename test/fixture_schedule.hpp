#include <gtest/gtest.h>
#include <thread>

#include "schedule.hpp"

template<typename Base>
class FixtureSchedule
  : public Base
{
public:
  FixtureSchedule()
    : sch(std::make_shared<amphisbaena::Schedule>(ios))
  {
  }

public:
  void SetUp()
  {
    work_ = std::make_unique<boost::asio::io_service::work>(ios);
    th_ = std::thread([this] { ios.run(); });
  }

  void TearDown()
  {
    work_.reset();
    th_.join();
  }

public:
  void stop() { ios.stop(); }

private:
  std::unique_ptr<boost::asio::io_service::work> work_;
  std::thread th_;

protected:
  boost::asio::io_service ios;
  std::shared_ptr<amphisbaena::Schedule> sch;
};

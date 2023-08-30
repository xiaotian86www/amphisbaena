#pragma once

#include <array>
#include <boost/asio/io_service.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <filesystem>
#include <memory>
#include <string_view>

#include "connection.hpp"
#include "schedule.hpp"
#include "session.hpp"

using namespace boost::asio::local;

namespace amphisbaena {
class UdsConnection : public Connection
{
public:
  UdsConnection(ScheduleRef sch,
                CoroutineRef co,
                MessageHandler& message_handler,
                stream_protocol::socket sock);

public:
  void send(std::string_view data) override;

  bool recv() override;

  void close() override;

private:
  stream_protocol::socket sock_;
  std::array<char, 1024> data_;
};

}
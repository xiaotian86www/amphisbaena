#pragma once

#include <array>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <filesystem>
#include <memory>
#include <string_view>

#include "connection.hpp"
#include "schedule.hpp"
#include "session.hpp"

using namespace boost::asio::ip;

namespace amphisbaena {
class TcpConnection : public Connection
{
public:
  TcpConnection(ScheduleRef sch,
                CoroutineRef co,
                MessageHandler& message_handler,
                tcp::socket sock);

public:
  void send(std::string_view data) override;

  bool recv() override;

  void close() override;

private:
  tcp::socket sock_;
  std::array<char, 1024> data_;
};

}
#include "asio_schedule.hpp"
#include "schedule.hpp"

#include <array>
#include <boost/asio/local/stream_protocol.hpp>
#include <memory>

using namespace boost::asio::local;

namespace translator {
class HttpServer : public std::enable_shared_from_this<HttpServer>
{
public:
  HttpServer(std::shared_ptr<Schedule> sch);
  ~HttpServer();

private:
  void do_accept(ScheduleRef sch);

  void do_read(std::shared_ptr<stream_protocol::socket> sock, ScheduleRef sch);

private:
  std::shared_ptr<Schedule> sch_;
  stream_protocol::acceptor acceptor_;
};
}
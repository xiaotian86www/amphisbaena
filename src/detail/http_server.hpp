#include "detail/asio_schedule.hpp"
#include "schedule.hpp"

#include <array>
#include <boost/asio/local/stream_protocol.hpp>
#include <memory>

using namespace boost::asio::local;

namespace translator {
class HttpServer : public std::enable_shared_from_this<HttpServer>
{
public:
  HttpServer(std::shared_ptr<AsioSchedule> sch);
  ~HttpServer();

private:
  void do_accept(std::shared_ptr<Coroutine> sch);

  void do_read(std::shared_ptr<stream_protocol::socket> sock, std::shared_ptr<Coroutine> sch);

private:
  std::shared_ptr<AsioSchedule> sch_;
  stream_protocol::acceptor acceptor_;
};
}
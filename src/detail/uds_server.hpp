#include "detail/asio_schedule.hpp"
#include "schedule.hpp"

#include <array>
#include <boost/asio/local/stream_protocol.hpp>
#include <memory>

using namespace boost::asio::local;

namespace translator {
class UDSServer : public std::enable_shared_from_this<UDSServer>
{
public:
  UDSServer(std::shared_ptr<AsioSchedule> sch);
  ~UDSServer();

protected:
  virtual void on_data(const char* data, std::size_t data_len);

private:
  void do_accept(std::shared_ptr<Coroutine> sch);

  void do_read(std::shared_ptr<stream_protocol::socket> sock,
               std::shared_ptr<Coroutine> sch);

private:
  std::shared_ptr<AsioSchedule> sch_;
  stream_protocol::acceptor acceptor_;
};
}
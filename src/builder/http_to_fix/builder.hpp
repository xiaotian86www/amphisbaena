
#include "environment.hpp"
#include "message.hpp"

namespace translator {
namespace builder {
class HttpToFixBuilder
{
public:
  MessagePtr operator()(Environment& env, MessagePtr request);
};
}
}
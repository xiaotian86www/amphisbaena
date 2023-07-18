#include "fix_builder.hpp"
#include "environment.hpp"

namespace translator {
FixMessageBuilder::FixMessageBuilder(std::istream& is)
  : client_(is)
{
}

}
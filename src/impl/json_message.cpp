
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "json_message.hpp"

namespace translator {
void
JsonObject::from_string(std::string_view str)
{
  RapidDocument doc(&allocator_);
  rapidjson::ParseResult ok = doc.Parse(str.data(), str.size());
  if (!ok) {
    // TODO 解析异常
    printf("JSON parse error: %s (%lu)\n",
           rapidjson::GetParseError_En(ok.Code()),
           ok.Offset());

    return;
  }

  value_.Swap(doc);
}

std::string
JsonObject::to_string() const
{
  rapidjson::StringBuffer sb;
  rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

  value_.Accept(writer);
  return std::string(sb.GetString(), sb.GetLength());
}
}
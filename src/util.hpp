#pragma once

#include <string_view>
#include <vector>

namespace amphisbaena {
namespace util {
inline std::vector<std::string_view>
split(std::string_view str, std::string_view split)
{
  std::vector<std::string_view> result;
  std::size_t pos = 0, new_pos = 0;

  if (str.empty()) return result;

  while (true) {
    new_pos = str.find(split, pos);
    if (new_pos == str.npos) {
      result.push_back(str.substr(pos));
      break;
    }

    if (pos != new_pos) {
      result.push_back(str.substr(pos, new_pos - pos));
    }

    pos = new_pos + 1;
  }

  return result;
}
}
}
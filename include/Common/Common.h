#ifndef CSSJIT_COMMON_H
#define CSSJIT_COMMON_H
#include <map>
#include <string>

namespace cssjit {
  using CSSMap = std::map<std::string,
    std::vector<std::pair<std::string, std::string>>>;
  int64_t hash(char *key) {
    std::string str(key);
    return std::hash<std::string>()(str);
  }
}

#endif

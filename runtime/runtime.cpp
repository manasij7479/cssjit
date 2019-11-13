#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include "../include/Parser/Parser.h"

extern"C" void match(char *str);

void traverse(mm::SyntaxTree &St) {
  if (St.Node == "tagid") {
    std::vector<std::string> toMatch;
    auto tagname = St.getFirstChild().Attributes["val"];
    toMatch.push_back(tagname);

    auto &&Attribs = St.getSecondChild();
    for (auto &&C : Attribs.Children) {
      if (C.getFirstChild().Attributes["val"] == "class") {
        auto classname = C.getSecondChild().getFirstChild().Attributes["val"];
        if (classname[0] == '\'' || classname[0] == '\"') {
          classname = classname.substr(1, classname.length() - 2);
        }
        toMatch.push_back(classname);
      }
    }

    for (auto &&str : toMatch) {
      std::cout << str << "\t";
      match(const_cast<char *>(str.c_str()));
    }
    std::cout << "\n";
  }
  for (auto &&C : St.Children) {
    traverse(C);
  }
}

int main(int argc, char **argv) {
  std::ifstream ins(argv[1]);
  std::ostringstream foo;
  foo << ins.rdbuf();

  std::string s = foo.str();

  mm::Stream in(s.c_str(), 0, s.length() - 1);

  auto St = mm::HTML(in);

  traverse(St);
  return 0;
}

extern"C" int64_t input() {
  int64_t n;
  std::cin >> n;
  return n;
}

extern"C" void printint(int64_t x) {
  std::cout << x;
}

extern"C" int32_t eq(char *x, char *y) {
  return strcmp(x, y) == 0;
}

extern "C" void printrule(char *key, char *value) {
  std::cout << key << ':' << value << ' ';
}

extern "C" int64_t hash(char *key) {
  std::string str(key);
  return std::hash<std::string>()(str);
}

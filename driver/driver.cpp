#include <iostream>
#include <map>
#include <sstream>
#include <fstream>
#include "Codegen/Codegen.h"
#include "Parser/Parser.h"

int main(int argc, char** argv) {
  std::ifstream ins(argv[2]);
  std::ostringstream foo;
  foo << ins.rdbuf();

  std::string s = foo.str();

  mm::Stream in(s.c_str(), 0, s.length() - 1);

  auto St = mm::CSS(in);

  std::cout << "CSS AST:\n";
  St.dump(std::cout);

  cssjit::CSSMap Rules;

  for (auto &&C : St.Children) {
    auto Name = C.getFirstChild().Attributes["val"];
    for (auto &&Props : C.getSecondChild().Children) {
      auto Key = Props.getFirstChild().Attributes["val"];
      auto Value = Props.getSecondChild().getFirstChild().Attributes["val"];
      Rules[Name].push_back({Key, Value});
    }
  }


  cssjit::Codegen generator(Rules);
  auto TU = generator(argv[1]);
  if (!TU) {
    std::cerr << "Codegen failed.";
  }
  return 0;
}

#include <iostream>
#include <map>
#include <sstream>
#include <fstream>
#include "Codegen/Codegen.h"
#include "Parser/Parser.h"

/**
 * Prototype input format
 * Each line has a key and one or more values
 * all positive integers
 *
 * Example:
 * key1 value
 * key2 value1 value2
 *
 * Prototype output
 * Executable which reads a file with a bunch of keys
 * (until 0 is read) and sums up all the associated values
 */

int main(int argc, char** argv) {
//   std::map<int, int> data;
//   std::string line;
//   while(std::getline(std::cin, line)) {
//     std::istringstream in(line);
//     int key, value;
//     in >> key;
//     int sumOfValues = 0;
//     while (in >> value) {
//       sumOfValues += value;
//     }
//     data[key] = sumOfValues;
//   }
//
//     std::ifstream ins(argv[1]);
//   std::ostringstream foo;
//   foo << ins.rdbuf();


  std::ifstream ins(argv[2]);
  std::ostringstream foo;
  foo << ins.rdbuf();

  std::string s = foo.str();

  mm::Stream in(s.c_str(), 0, s.length() - 1);

  auto St = mm::CSS(in);

  std::cout << "HERE";
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

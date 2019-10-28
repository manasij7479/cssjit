#include <iostream>
#include <map>
#include <sstream>
#include "Codegen/Codegen.h"
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
  std::map<int, int> data;
  std::string line;
  while(std::getline(std::cin, line)) {
    std::istringstream in(line);
    int key, value;
    in >> key;
    int sumOfValues = 0;
    while (in >> value) {
      sumOfValues += value;
    }
    data[key] = sumOfValues;
  }

  for (auto P : data) {
    std::cout << P.first << P.second << "\n";
  }

  cssjit::Codegen generator(data);
  auto TU = generator(argv[1]);
  if (!TU) {
    std::cerr << "Codegen failed.";
  }
  return 0;
}

#include <iostream>
#include <map>
#include <fstream>
#include <sstream>

int main(int argc, char **argv) {  
  std::map<int, int> data;
  std::string line;
  std::ifstream fin1(argv[1]);
  while(std::getline(fin1, line)) {
    std::istringstream in(line);
    int key, value;
    in >> key;
    int sumOfValues = 0;
    while (in >> value) {
      sumOfValues += value;
    }
    data[key] = sumOfValues;
  }
  
  std::ifstream fin2(argv[2]);
  int64_t sum = 0;
  int input;
  while (fin2 >> input) {
    if (!input) {
      break;
    }
    if (data.find(input) != data.end()) {
      sum += data[input];
    }
  }
  std::cout << sum;
  return 0;
}

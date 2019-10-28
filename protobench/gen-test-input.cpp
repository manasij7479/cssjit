#include <iostream>
#include <cstdlib>
#include <ctime>
int main(int argc, char **argv) {
  int n = std::stoi(argv[1]);
  std::srand(std::time(nullptr));
  while(n--) {
    std::cout << std::max(1, std::rand() % 10) << " ";
  }
  std::cout << 0;
  return 0;
} 

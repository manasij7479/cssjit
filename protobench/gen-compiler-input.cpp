#include <iostream>
#include <cstdlib>
#include <ctime>
int main(int argc, char **argv) {
  int n = std::stoi(argv[1]);
  std::srand(std::time(nullptr));
  
  for (int i = 1; i < 9; ++i) {
    int n_ = n;
    std::cout << i << " ";
    while(n_--) {
      std::cout << std::max(0, std::rand() % 10) << " ";
    }
    std::cout << "\n";
  }
  
  return 0;
} 

cmake_minimum_required(VERSION 2.8.8)
add_executable(cssjit-driver driver.cpp)
target_link_libraries(cssjit-driver
  Codegen ${llvm_libs})

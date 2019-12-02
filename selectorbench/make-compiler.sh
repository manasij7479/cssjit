ninja -C ../build
../build/driver/cssjit-driver out.ll  $1 >/dev/null
cat out.ll
clang++ -Wno-override-module ../runtime/runtime.cpp out.ll -O3 -o compiler

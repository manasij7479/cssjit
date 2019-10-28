#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

extern int cssjit_main(void);
int main() {
  return cssjit_main();
}

int64_t input() {
  int64_t n;
  scanf("%"SCNi64, &n);
  return n;
}

void printint(int64_t x) {
  printf("%"PRIi64, x);
}

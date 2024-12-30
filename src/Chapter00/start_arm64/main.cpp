#include <cstdint>
#include <cstdio>

#include "func.h"

int main() {
  for (uint64_t i = 0; i < 10; i++) {
    uint64_t const sum = mySum(i);
    uint64_t const fibo = myFibo(i);
    printf("sum(%ld) = %ld, fibo(%ld) = %ld\n", i, sum, i, fibo);

    // TODO(): wrong answer may due to not follow the register saving requirements of the calling convention
    // printf("inline sum(%ld) = %ld, fibo(%ld) = %ld\n", i, mySum(i), i, myFibo(i));
  }
}
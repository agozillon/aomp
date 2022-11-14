#include "stdio.h"

/** Problem is map clause tries to read value
 * from N from the device and write it into
 * constant memory (constant generated by LLVM)
 */

#define ERROR_CHECK(value, expected)                                           \
  error = expected - value;                                                    \
  if (error) {                                                                 \
    return error;                                                              \
  }

const int N = 128;
int M = 1;

typedef struct A {
  int a;
} A_t;

int main(int argc, char **argv) {
  int error = 0;
  int tmp = 0;
  int tmp2 = 0;
  const int val = 42;

#pragma omp target teams distribute parallel for private(tmp) map(N)
  for (int i = 0; i < N; i++) {
    tmp += i;
    M = 42;
  }

  ERROR_CHECK(M, 1)
  ERROR_CHECK(tmp, 0)

#pragma omp target teams distribute parallel for private(tmp) map(N) map(M)
  for (int i = 0; i < N; i++) {
    tmp += i;
    M = 42;
  }

  ERROR_CHECK(M, 42)
  ERROR_CHECK(tmp, 0)

#pragma omp target teams distribute parallel for private(tmp) map(M) map(N)
  for (int i = 0; i < N; i++) {
    tmp += i;
    M = N;
  }

  ERROR_CHECK(M, N)

#pragma omp target teams distribute parallel for private(tmp) map(tofrom:N) map(tofrom:M)
  for (int i = 0; i < N; i++) {
    tmp += i;
    M = 32;
  }

  ERROR_CHECK(M, 32)

#pragma omp target teams distribute parallel for private(tmp) map(to:N) map(to:M)
  for (int i = 0; i < N; i++) {
    tmp += i;
    M = 21;
  }

  ERROR_CHECK(M, 32)

#pragma omp target teams distribute parallel for private(tmp) map(val) map(N, M)
  for (int i = 0; i < N; i++) {
    tmp += i;
    M = val;
  }

  ERROR_CHECK(M, val)

#pragma omp target teams distribute parallel for private(tmp) map(val, N, M)
  for (int i = 0; i < N; i++) {
    tmp += i;
    M = val + N;
  }
  ERROR_CHECK(M, val + N)

  fprintf(stderr, "Passed\n");

  return 0;
}
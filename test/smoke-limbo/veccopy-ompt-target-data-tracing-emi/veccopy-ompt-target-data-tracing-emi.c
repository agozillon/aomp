#include <stdio.h>
#include <assert.h>
#include <omp.h>

#include "callbacks.h"

#define N 100000

static int start_trace();
static int flush_trace();
static int stop_trace();

#pragma omp declare target
int c[N];
#pragma omp end declare target

int main() {
  int a[N];
  int b[N];

  int i;

  for (i = 0; i < N; i++)
    a[i] = 0;

  for (i = 0; i < N; i++)
    b[i] = i;

  for (i = 0; i < N; i++)
    c[i] = 0;

  start_trace();

#pragma omp target enter data map(to : a)
  flush_trace();

#pragma omp target parallel for
  {
    for (int j = 0; j < N; j++)
      a[j] = b[j];
  }
#pragma omp target exit data map(from : a)

  flush_trace();
  stop_trace();

  start_trace();

#pragma omp target parallel for map(alloc : c)
  {
    for (int j = 0; j < N; j++)
      c[j] = 2 * j + 1;
  }
#pragma omp target update from(c) nowait
#pragma omp barrier

  stop_trace();

  int rc = 0;
  for (i = 0; i < N; i++) {
    if (a[i] != i) {
      rc++;
      printf("Wrong value: a[%d]=%d\n", i, a[i]);
    }
  }

  for (i = 0; i < N; i++) {
    if (c[i] != 2 * i + 1) {
      rc++;
      printf("Wrong value: c[%d]=%d\n", i, c[i]);
    }
  }

  if (!rc)
    printf("Success\n");

  return rc;
}

/// CHECK-NOT: Callback Target EMI:
/// CHECK-NOT: device_num=-1

/// CHECK: Callback Init:
/// CHECK: Callback Load:

/// CHECK: Callback Target EMI: kind=2 endpoint=1
/// CHECK-DAG: Callback DataOp EMI: endpoint=1 optype=1
/// CHECK-DAG: Callback DataOp EMI: endpoint=2 optype=1
/// CHECK-DAG: Callback DataOp EMI: endpoint=1 optype=2
/// CHECK-DAG: Callback DataOp EMI: endpoint=2 optype=2
/// CHECK-DAG: Callback Target EMI: kind=2 endpoint=2

/// CHECK-DAG: Record Target task
/// CHECK-DAG: Record Target data op
/// CHECK-DAG: Record Target data op
/// CHECK-DAG: Record Target task

/// CHECK: Callback Target EMI: kind=1 endpoint=1
/// CHECK-DAG: Callback DataOp EMI: endpoint=1 optype=1
/// CHECK-DAG: Callback DataOp EMI: endpoint=2 optype=1
/// CHECK-DAG: Callback DataOp EMI: endpoint=1 optype=2
/// CHECK-DAG: Callback DataOp EMI: endpoint=2 optype=2
/// CHECK-DAG: Callback Submit EMI: endpoint=1  req_num_teams=1
/// CHECK-DAG: Callback Submit EMI: endpoint=2  req_num_teams=1
/// CHECK-DAG: Callback DataOp EMI: endpoint=1 optype=3
/// CHECK-DAG: Callback DataOp EMI: endpoint=2 optype=3
/// CHECK-DAG: Callback DataOp EMI: endpoint=1 optype=4
/// CHECK-DAG: Callback DataOp EMI: endpoint=2 optype=4
/// CHECK-DAG: Callback Target EMI: kind=1 endpoint=2
/// CHECK-DAG: Callback Target EMI: kind=3 endpoint=1
/// CHECK-DAG: Callback DataOp EMI: endpoint=1 optype=3
/// CHECK-DAG: Callback DataOp EMI: endpoint=2 optype=3
/// CHECK-DAG: Callback DataOp EMI: endpoint=1 optype=4
/// CHECK-DAG: Callback DataOp EMI: endpoint=2 optype=4
/// CHECK-DAG: Callback Target EMI: kind=3 endpoint=2

/// CHECK-DAG: Record Target task
/// CHECK-DAG: Record Target data op
/// CHECK-DAG: Record Target data op
/// CHECK-DAG: Record Target kernel
/// CHECK-DAG: Record Target data op
/// CHECK-DAG: Record Target data op
/// CHECK-DAG: Record Target task

/// CHECK-DAG: Record Target task
/// CHECK-DAG: Record Target data op
/// CHECK-DAG: Record Target data op
/// CHECK-DAG: Record Target task

/// CHECK: Callback Target EMI: kind=1 endpoint=1
/// CHECK-DAG: Callback Submit EMI: endpoint=1  req_num_teams=1
/// CHECK-DAG: Callback Submit EMI: endpoint=2  req_num_teams=1
/// CHECK-DAG: Callback Target EMI: kind=1 endpoint=2
/// CHECK-DAG: Callback Target EMI: kind=4 endpoint=1
/// CHECK-DAG: Callback DataOp EMI: endpoint=1 optype=3
/// CHECK-DAG: Callback DataOp EMI: endpoint=2 optype=3
/// CHECK-DAG: Callback Target EMI: kind=4 endpoint=2

/// CHECK-DAG: Record Target task
/// CHECK-DAG: Record Target kernel
/// CHECK-DAG: Record Target task

/// CHECK-DAG: Record Target task
/// CHECK-DAG: Record Target data op
/// CHECK-DAG: Record Target task

/// CHECK: Callback Fini:

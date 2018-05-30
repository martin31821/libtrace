#pragma once

#include <stdio.h>
#include <stdatomic.h>

struct _trace_t {
  FILE *trace_file;
  atomic_size_t invocation_idx;
};

#define _GNU_SOURCE
#include "trace.h"
#include "trace_int.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

trace_t* trace_init(const char *fname) {
  FILE *file = fopen(fname, "w");
  if (!file) {
    return NULL;
  }
  // disable any buffering
  setbuf(file, NULL);

  trace_t *trace = malloc(sizeof(trace_t));
  if (!trace) {
    fclose(file);
    return trace;
  }
  trace->trace_file = file;
  trace->invocation_idx = 0;
  const char start[4] = { 0xff, 0xff, 0xff, 0xff };
  fwrite(start, 4, 1, trace->trace_file);
  return trace;
}

int trace_close(trace_t *t) {
  if (!t) return 0;

  return fflush(t->trace_file) | fclose(t->trace_file);
}

size_t trace_call_entry(trace_t *trace, const invocation_t *invocation) {
  if (!trace || !invocation) return (size_t)-1;
  size_t id = atomic_fetch_add_explicit(&trace->invocation_idx, 1, memory_order_relaxed);
  const char *start = "CALL";
  fwrite(start, 4, 1, trace->trace_file);
  fwrite(&id, sizeof(size_t), 1, trace->trace_file);
  size_t mnamelen = strlen(invocation->method);
  fwrite(&mnamelen, sizeof(size_t), 1, trace->trace_file);
  fwrite(invocation->method, 1, mnamelen, trace->trace_file);
  fwrite(&invocation->num_params, sizeof(size_t), 1, trace->trace_file);
  for (size_t pid = 0; pid < invocation->num_params; pid++) {
    const value_t *param = invocation->params + pid;
    fwrite(&param->size, sizeof(size_t), 1, trace->trace_file);
    fwrite(param->data, 1, param->size, trace->trace_file);
  }
  fflush(trace->trace_file);
  return id;
}

void trace_call_exit(trace_t *trace, size_t invocation, const value_t *retval) {
  if (!trace || invocation == (size_t)-1) return;
  const char *start = "EXIT";
  fwrite(start, 4, 1, trace->trace_file);
  fwrite(&invocation, sizeof(size_t), 1, trace->trace_file);
  uint8_t has_ret = retval ? 1 : 0;
  fwrite(&has_ret, 1, 1, trace->trace_file);
  if (has_ret) {
    fwrite(&retval->size, sizeof(size_t), 1, trace->trace_file);
    fwrite(retval->data, 1, retval->size, trace->trace_file);
  }
  fflush(trace->trace_file);
}

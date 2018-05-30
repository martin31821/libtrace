#pragma once
#include <stdint.h>
#include <stddef.h>

#define MAX_PARAM_NUM (16)

typedef struct _trace_t trace_t;

typedef struct {
  size_t size;
  void* data;
} value_t;

typedef struct {
  // Name of the method. must match the actual implementation name, in order for the player to work
  const char *method;
  size_t num_params;
  value_t params[MAX_PARAM_NUM];
} invocation_t;

// Initialize the trace library.
// @returns null if a failure occured, otherwise a valid pointer.
// Output file MUST NOT be null
trace_t* trace_init(const char *output_file);

// Closes the given instance of the trace library
// @returns 0 on success, any nonzero value means failed.
// It is safe to pass null as trace parameter
int trace_close(trace_t *trace);

// Call this function a the entrance of a function you want to instrument
// This will return an identifier to allow parallel calls
size_t trace_call_entry(trace_t *trace, const invocation_t *invocation);

// Call this function a the exit of a function you want to instrument
// giving the value trace_call_entry gave you and the return va lue.
// If your function is void, the return value must be NULL
void trace_call_exit(trace_t *trace, size_t invocation, const value_t *return_val);

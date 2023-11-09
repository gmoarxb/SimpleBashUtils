#include "safe.h"

#include <stdlib.h>

void* safe_malloc(const size_t size) {
  void* ret_ptr = malloc(size);
  if (ret_ptr == NULL) {
    fprintf(stderr, "%zu: Memory allocation error.\n", size);
    exit(EXIT_FAILURE);
  }
  return ret_ptr;
}

void* safe_realloc(void* ptr, size_t size) {
  void* ret_ptr = realloc(ptr, size);
  if (ret_ptr == NULL) {
    fprintf(stderr, "%zu: Memory reallocation error.\n", size);
    exit(EXIT_FAILURE);
  }
  return ret_ptr;
}

FILE* safe_fopen(const char* filename, const char* modes) {
  FILE* ret_ptr = fopen(filename, modes);
  if (ret_ptr == NULL) {
    fprintf(stderr, "%s: No such file or directory\n", filename);
    exit(EXIT_FAILURE);
  }
  return ret_ptr;
}
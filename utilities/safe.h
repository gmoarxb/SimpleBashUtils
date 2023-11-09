#ifndef SAFE_H
#define SAFE_H

#include <stdio.h>

void* safe_malloc(const size_t size);
void* safe_realloc(void* ptr, size_t size);
FILE* safe_fopen(const char* filename, const char* modes);

#endif  // SAFE_H
#include "string_array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void string_array_create(StringArray* const string_array) {
  if (string_array) {
    string_array->size = 0;
    string_array->max_size = STRING_ARRAY_CREATE_SIZE;
    string_array->strings = calloc(STRING_ARRAY_CREATE_SIZE, sizeof(char*));
    if (!string_array->strings) {
      string_array_handle_error(string_array);
    }
  }
}

void string_array_add(StringArray* const string_array, char* string) {
  if (string_array != NULL) {
    if (string_array->size == string_array->max_size) {
      string_array->max_size *= STRING_ARRAY_REALLOC_COEFFICIENT;
      char** temp = realloc(string_array->strings, string_array->max_size);
      if (temp != NULL) {
        string_array->strings = temp;
      } else {
        string_array_handle_error(string_array);
      }
    }
    string_array->strings[string_array->size] = malloc(sizeof(char) * (strlen(string) + 1));
    strcpy(string_array->strings[string_array->size], string);
    ++string_array->size;
  }
}

void string_array_destroy(StringArray* const string_array) {
  if (string_array != NULL) {
    for (unsigned i = 0; i < string_array->size; ++i) {
      free(string_array->strings[i]);
      string_array->strings[i] = NULL;
    }
    string_array->max_size = 0;
    string_array->size = 0;
    free(string_array->strings);
    string_array->strings = NULL;
  }
}

void string_array_handle_error(StringArray* const string_array) {
  string_array_destroy(string_array);
  fprintf(stderr, "%s", "StringArray memory error!\n");
  exit(EXIT_FAILURE);
}
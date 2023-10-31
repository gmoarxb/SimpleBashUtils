#ifndef STRING_ARRAY
#define STRING_ARRAY

#define STRING_ARRAY_CREATE_SIZE 128
#define STRING_ARRAY_REALLOC_COEFFICIENT 2

typedef struct StringArray {
  char** strings;
  unsigned size;
  unsigned max_size;
} StringArray;

void string_array_create(StringArray* const string_array);
void string_array_add(StringArray* const, char* string);
void string_array_destroy(StringArray* const string_array);
void string_array_handle_error(StringArray* const string_array);

#endif  // STRING_ARRAY





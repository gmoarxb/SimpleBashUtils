#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void geberate_gibberish(FILE* test_file) {
  int count = 64;
  while (count--) {
    char byte = rand() % 256;
    fwrite(&byte, sizeof(char), 1, test_file);
  }
}

const char spaces[] = {0x0c, 0x0a, 0x0d, 0x09, 0x0b};

void geberate_spaces(FILE* test_file) {
  int s_count = 16;
  while (s_count--) {
    fwrite(spaces + (rand() % sizeof(spaces)), sizeof(char), 1, test_file);
  }
}

void geberate_lfd(FILE* test_file) {
  int s_count = 4;
  while (s_count--) {
    fwrite(spaces + 1, sizeof(char), 1, test_file);
  }
}

void new_file(const char* fname) {
  FILE* test_file = fopen(fname, "w");
  if (!strcmp(fname, "empty")) {
  } else if (!strcmp(fname, "lfd")) {
    geberate_lfd(test_file);
  } else {
    if (rand() % 2) geberate_lfd(test_file);
    if (rand() % 2) geberate_spaces(test_file);
      geberate_gibberish(test_file);
    if (rand() % 2) geberate_lfd(test_file);
  }
  fclose(test_file);
}

int main(int argc, char* argv[]) {
  srand(time(NULL));
  while (--argc) {
    new_file(argv[argc]);
  }
  return 0;
}
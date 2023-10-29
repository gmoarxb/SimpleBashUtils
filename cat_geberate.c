#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void geberate_gibberish(FILE* test_file) {
  int count = 256;
  while (count--) {
    int bytes = (count % 2) ? rand() : -rand();
    fwrite(&bytes, sizeof(int), 1, test_file);
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
  FILE* test_file = fopen(fname, "wb");
  int spaces_begin = rand() % 2;
  int spaces_end = rand() % 2;
  if (spaces_begin) {
    geberate_lfd(test_file);
  }
  geberate_spaces(test_file);
  geberate_gibberish(test_file);
  geberate_spaces(test_file);
  geberate_gibberish(test_file);
  geberate_spaces(test_file);
  if (spaces_end) {
    geberate_lfd(test_file);
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
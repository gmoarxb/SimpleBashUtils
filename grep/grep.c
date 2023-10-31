// #include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_array.h"

#define GETOPT_OPTIONS_END -1
#define FOPEN_READ_MODE "r"
#define GET_LINE_START_SIZE 128
#define GET_LINE_REALLOC_COEFFICIENT 2

static const char SHORT_OPTIONS[] = "e:f:isvnholc";

static const struct option LONG_OPTIONS[] = {
    {"regexp", 1, NULL, 'e'},
    {"file", 1, NULL, 'f'},
    {"ignore-case", 0, NULL, 'i'},
    {"no-messages", 0, NULL, 's'},
    {"invert-match", 0, NULL, 'v'},
    {"line-number", 0, NULL, 'n'},
    {"no-filename", 0, NULL, 'h'},
    {"only-matching", 0, NULL, 'o'},
    {"files-with-matches", 0, NULL, 'l'},
    {"count", 0, NULL, 'c'},
    {"help", 0, NULL, 0},
    {NULL, 0, NULL, 0}};

typedef struct Options {
  bool e;
  bool f;
  bool i;
  bool s;
  bool v;
  bool n;
  bool h;
  bool o;
  bool l;
  bool c;
  StringArray patterns;
  StringArray patterns_files;
} Options;

static void init_options(int argc, char* const argv[], Options* const options);
static void free_options(Options* const options);
static void set_option(const char option, Options* const options);
static void print_help();
static void print_invalid_option();
static void free_options(Options* const options);
static void add_patterns(char* const patterns, Options* const options);
static void add_patterns_from_file(char* const file_path,
                                   Options* const options);
static void print_invalid_file(const char* const file_name);
static char* get_line_from_file(FILE* file);

int main(int argc, char* argv[]) {
  Options options = {0};
  init_options(argc, argv, &options);
  for (unsigned i = 0; i < options.patterns.size; ++i) {
    puts(options.patterns.strings[i]);
  }
  free_options(&options);
  return 0;
}

static void init_options(int argc, char* const argv[], Options* const options) {
  string_array_create(&options->patterns);
  string_array_create(&options->patterns_files);
  int long_options_index = 0;
  char current_option =
      getopt_long(argc, argv, SHORT_OPTIONS, LONG_OPTIONS, &long_options_index);
  while (current_option != GETOPT_OPTIONS_END) {
    set_option(current_option, options);
    current_option = getopt_long(argc, argv, SHORT_OPTIONS, LONG_OPTIONS,
                                 &long_options_index);
  }
}

static void free_options(Options* const options) {
  string_array_destroy(&options->patterns);
  string_array_destroy(&options->patterns_files);
}

static void set_option(const char option, Options* const options) {
  switch (option) {
    case 'e':
      options->e = true;
      add_patterns(optarg, options);
      break;
    case 'f':
      add_patterns_from_file(optarg, options);
      options->f = true;
      break;
    case 'i':
      options->i = true;
      break;
    case 's':
      options->s = true;
      break;
    case 'v':
      options->v = true;
      break;
    case 'n':
      options->n = true;
      break;
    case 'h':
      options->h = true;
      break;
    case 'o':
      options->o = true;
      break;
    case 'l':
      options->l = true;
      break;
    case 'c':
      options->c = true;
      break;
    case 0:
      print_help();
      break;
    case '?':
    default:
      print_invalid_option();
  }
}

static void print_help() {
  fprintf(stdout,
          "Usage: grep [OPTION]... PATTERNS [FILE]...\n"
          "Search for PATTERNS in each FILE.\n");
  exit(EXIT_FAILURE);
}

static void print_invalid_option() {
  fprintf(stderr, "Try 'grep --help' for more information.\n");
  exit(EXIT_FAILURE);
}

static void add_patterns(char* const patterns, Options* const options) {
  char* pattern = strtok(patterns, "\n");
  while (pattern) {
    string_array_add(&options->patterns, pattern);
    pattern = strtok(NULL, "\n");
  }
}

static void add_patterns_from_file(char* const file_path,
                                   Options* const options) {
  char* pattern = NULL;
  FILE* pattern_file = fopen(file_path, FOPEN_READ_MODE);
  if (pattern_file) {
    while (!feof(pattern_file)) {
      pattern = get_line_from_file(pattern_file);
      string_array_add(&options->patterns, pattern);
      free(pattern);
    }
  } else {
    print_invalid_file(file_path);
  }
}

static char* get_line_from_file(FILE* file) {
  unsigned size = 0;
  unsigned max_size = GET_LINE_START_SIZE;
  char* line = malloc(sizeof(char) * max_size);
  if (line == NULL) {
    fprintf(stderr, "%s", "get_line memory error!\n");
    exit(EXIT_FAILURE);
  }
  char current_symbol = fgetc(file);
  while (!feof(file) && current_symbol != '\n') {
    line[size++] = current_symbol;
    current_symbol = fgetc(file);
    if (size == max_size) {
      max_size *= GET_LINE_REALLOC_COEFFICIENT;
      char* temp = realloc(line, max_size);
      if (temp != NULL) {
        line = temp;
      } else {
        fprintf(stderr, "%s", "get_line memory error!\n");
        exit(EXIT_FAILURE);
      }
    }
  }
  line[size] = '\0';
  return line;
}

static void print_invalid_file(const char* const file_name) {
  fprintf(stderr, "cat: %s: No such file or directory\n", file_name);
}
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GETOPT_OPTIONS_END -1
#define FOPEN_READ_MODE "r"

#define PATTERNS_INIT_CAPACITY 128
#define PATTERNS_CAPACITY_ADDEND 128

#define FILE_BUFFER_INIT_CAPACITY 128
#define FILE_BUFFER_CAPACITY_MULTIPLIER 2

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
  struct {
    char** data;
    unsigned size;
    unsigned capacity;
  } patterns;
} Options;

static void init_options(int argc, char* const argv[], Options* const options);
static void free_options(Options* const options);
static void set_option(const char option, Options* const options);
static void print_help();
static void print_invalid_option();
static void free_options(Options* const options);
static void add_patterns(const char* const patterns, Options* const options);
static void add_patterns_from_file(char* const file_path,
                                   Options* const options);
static void print_invalid_file(const char* const file_name);
static void get_patterns_from_file(FILE* pattern_file, char* file_buffer);
static void add_pattern(const char* const pattern, Options* const options);

int main(int argc, char* argv[]) {
  Options options = {0};
  init_options(argc, argv, &options);
  for (unsigned i = 0; i < options.patterns.size; ++i) {
    puts(options.patterns.data[i]);
  }
  free_options(&options);
  return 0;
}

static void init_options(int argc, char* const argv[], Options* const options) {
  options->patterns.capacity = PATTERNS_INIT_CAPACITY;
  options->patterns.size = 0;
  options->patterns.data = malloc(sizeof(char*) * PATTERNS_INIT_CAPACITY);

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
  for (unsigned i = 0; i < options->patterns.size; ++i) {
    free(options->patterns.data[i]);
  }
  free(options->patterns.data);
}

static void set_option(const char option, Options* const options) {
  switch (option) {
    case 'e':
      options->e = true;
      add_patterns(optarg, options);
      break;
    case 'f':
      options->f = true;
      add_patterns_from_file(optarg, options);
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

static void add_patterns(const char* const patterns, Options* const options) {
  char* temp_patterns = malloc(sizeof(char) * strlen(patterns) + sizeof(char));
  if (temp_patterns != NULL) {
    strcpy(temp_patterns, patterns);
    char* token = strtok(temp_patterns, "\n");
    while (token != NULL) {
      add_pattern(token, options);
      token = strtok(NULL, "\n");
    }
    free(temp_patterns);
  } else {
    exit(EXIT_FAILURE);
  }
}

static void add_pattern(const char* const pattern, Options* const options) {
  if (options->patterns.size == options->patterns.capacity) {
    options->patterns.capacity += PATTERNS_CAPACITY_ADDEND;
    char** temp = realloc(options->patterns.data, options->patterns.capacity);
    if (temp != NULL) {
      options->patterns.data = temp;
    } else {
      exit(EXIT_FAILURE);
    }
  }
  options->patterns.data[options->patterns.size] =
      malloc(sizeof(char) * strlen(pattern) + sizeof(char));
  strcpy(options->patterns.data[options->patterns.size], pattern);
  ++options->patterns.size;
}

static void add_patterns_from_file(char* const file_path,
                                   Options* const options) {
  FILE* pattern_file = fopen(file_path, FOPEN_READ_MODE);
  if (pattern_file != NULL) {
    char* file_buffer = malloc(sizeof(char) * FILE_BUFFER_INIT_CAPACITY);
    if (file_buffer != NULL) {
      get_patterns_from_file(pattern_file, file_buffer);
      add_patterns(file_buffer, options);
      free(file_buffer);
    } else {
      exit(EXIT_FAILURE);
    }
  } else {
    print_invalid_file(file_path);
  }
  fclose(pattern_file);
}

static void get_patterns_from_file(FILE* pattern_file, char* file_buffer) {
  unsigned file_buffer_size = 0;
  unsigned file_buffer_capacity = FILE_BUFFER_INIT_CAPACITY;
  int symbol = fgetc(pattern_file);
  while (!feof(pattern_file)) {
    file_buffer[file_buffer_size] = symbol;
    ++file_buffer_size;
    if (file_buffer_size == file_buffer_capacity) {
      file_buffer_capacity *= FILE_BUFFER_CAPACITY_MULTIPLIER;
      char* temp = realloc(file_buffer, file_buffer_capacity);
      if (temp != NULL) {
        file_buffer = temp;
      } else {
        free(file_buffer);
        exit(EXIT_FAILURE);
      }
    }
    symbol = fgetc(pattern_file);
  }
  file_buffer[file_buffer_size] = '\0';
}

static void print_invalid_file(const char* const file_name) {
  fprintf(stderr, "grep: %s: No such file or directory\n", file_name);
}
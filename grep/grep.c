#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GETOPT_END -1
#define FOPEN_READ "r"

#define PATTERNS_INIT 128
#define PATTERNS_ADD 128

#define BUFFER_INIT 128
#define BUFFER_MULT 2

static const char SHORTOPTS[] = "e:f:isvnholc";

static const struct option LONGOPTS[] = {
    {"regexp", required_argument, NULL, 'e'},
    {"file", required_argument, NULL, 'f'},
    {"ignore-case", no_argument, NULL, 'i'},
    {"no-messages", no_argument, NULL, 's'},
    {"invert-match", no_argument, NULL, 'v'},
    {"line-number", no_argument, NULL, 'n'},
    {"no-filename", no_argument, NULL, 'h'},
    {"only-matching", no_argument, NULL, 'o'},
    {"files-with-matches", no_argument, NULL, 'l'},
    {"count", no_argument, NULL, 'c'},
    {"help", no_argument, NULL, 0},
    {NULL, 0, NULL, 0}};

typedef struct Patterns {
  char** data;
  size_t cur_size;
  size_t max_size;
} Patterns;

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
  Patterns patts;
} Options;

static void init_options(int argc, char* const argv[], Options* const opts);
static void init_patterns(Patterns* const patts);
static void process_files(int file_cnt, char* const file_path[],
                          const Options* const opts);

static void free_options(Options* const opts);
static void print_help();
static void print_invalid_file(const char* const filename);

static void* safe_malloc(const size_t size);
static void set_option(const char opt, Options* const opts);
static void add_patterns_from_string(const char* const str,
                                     Patterns* const patts);

static void add_patterns_from_file(char* const filename, Patterns* const patts);
static FILE* safe_fopen(const char* filename, const char* modes);
static void print_invalid_option();
static void* safe_realloc(void* ptr, size_t size);
static void add_pattern(const char* const patt, Patterns* const patts);
static void get_patterns_from_file(FILE* file, char* buffer);

int main(int argc, char* argv[]) {
  Options opts = {0};
  init_options(argc, argv, &opts);
  // process_files(argc - optind, argv + optind, &opts);
  for (size_t i = 0; i < opts.patts.cur_size; ++i) {
    puts(opts.patts.data[i]);
  }
  free_options(&opts);
  return 0;
}

static void init_options(int argc, char* const argv[], Options* const opts) {
  init_patterns(&opts->patts);
  int longind = 0;
  char cur_opt = getopt_long(argc, argv, SHORTOPTS, LONGOPTS, &longind);
  while (cur_opt != GETOPT_END) {
    set_option(cur_opt, opts);
    cur_opt = getopt_long(argc, argv, SHORTOPTS, LONGOPTS, &longind);
  }
}

static void free_options(Options* const opts) {
  for (size_t i = 0; i < opts->patts.cur_size; ++i) {
    free(opts->patts.data[i]);
  }
  free(opts->patts.data);
}

static void init_patterns(Patterns* const patts) {
  patts->cur_size = 0;
  patts->max_size = PATTERNS_INIT;
  patts->data = safe_malloc(sizeof(char*) * patts->max_size);
}

static void* safe_malloc(const size_t size) {
  void* ret_ptr = malloc(size);
  if (ret_ptr == NULL) {
    fprintf(stderr, "Memory allocation error.\n");
    exit(EXIT_FAILURE);
  }
  return ret_ptr;
}

static void set_option(const char opt, Options* const opts) {
  switch (opt) {
    case 'e':
      opts->e = true;
      add_patterns_from_string(optarg, &opts->patts);
      break;
    case 'f':
      opts->f = true;
      add_patterns_from_file(optarg, &opts->patts);
      break;
    case 'i':
      opts->i = true;
      break;
    case 's':
      opts->s = true;
      break;
    case 'v':
      opts->v = true;
      break;
    case 'n':
      opts->n = true;
      break;
    case 'h':
      opts->h = true;
      break;
    case 'o':
      opts->o = true;
      break;
    case 'l':
      opts->l = true;
      break;
    case 'c':
      opts->c = true;
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

static void add_patterns_from_string(const char* const str,
                                     Patterns* const patts) {
  char* temp_patts = safe_malloc(sizeof(char) * strlen(str) + sizeof(char));
  strcpy(temp_patts, str);
  char* token = strtok(temp_patts, "\n");
  while (token != NULL) {
    add_pattern(token, patts);
    token = strtok(NULL, "\n");
  }
  free(temp_patts);
}

static void add_pattern(const char* const patt, Patterns* const patts) {
  if (patts->cur_size == patts->max_size) {
    patts->max_size += PATTERNS_ADD;
    patts->data = safe_realloc(patts->data, patts->max_size);
  }
  patts->data[patts->cur_size] =
      safe_malloc(sizeof(char) * strlen(patt) + sizeof(char));
  strcpy(patts->data[patts->cur_size++], patt);
}

static void* safe_realloc(void* ptr, size_t size) {
  void* ret_ptr = realloc(ptr, size);
  if (ret_ptr == NULL) {
    fprintf(stderr, "Memory reallocation error.\n");
    exit(EXIT_FAILURE);
  }
  return ret_ptr;
}

static void add_patterns_from_file(char* const filename,
                                   Patterns* const patts) {
  FILE* file = safe_fopen(filename, FOPEN_READ);
  char* buffer = safe_malloc(sizeof(char) * BUFFER_INIT);
  get_patterns_from_file(file, buffer);
  add_patterns_from_string(buffer, patts);
  free(buffer);
  fclose(file);
}

static FILE* safe_fopen(const char* filename, const char* modes) {
  FILE* ret_ptr = fopen(filename, modes);
  if (ret_ptr == NULL) {
    print_invalid_file(filename);
    exit(EXIT_FAILURE);
  }
  return ret_ptr;
}

static void get_patterns_from_file(FILE* file, char* buffer) {
  unsigned size = 0;
  unsigned max_size = BUFFER_INIT;
  int symbol = fgetc(file);
  while (!feof(file)) {
    buffer[size++] = symbol;
    if (size == max_size) {
      max_size *= BUFFER_MULT;
      char* temp = safe_realloc(buffer, max_size);
      buffer = temp;
    }
    symbol = fgetc(file);
  }
  buffer[size] = '\0';
}

static void print_invalid_file(const char* const filename) {
  fprintf(stderr, "grep: %s: No such file or directory\n", filename);
}
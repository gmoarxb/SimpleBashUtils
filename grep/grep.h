#ifndef GREP_H
#define GREP_H

#include <stdbool.h>
#include <stdio.h>

#define GETOPT_END -1
#define FOPEN_READ "r"

#define PATTERNS_INIT 128
#define PATTERNS_ADD 128

#define BUFFER_INIT 128
#define BUFFER_MULT 2

struct Patterns {
  char** data;
  size_t cur_size;
  size_t max_size;
};

struct Options {
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
  struct Patterns patts;
};

typedef struct Patterns Patterns;
typedef struct Options Options;

static void options_init(Options* const opts, int argc, char* const argv[]);
static void options_free(Options* const opts);
static void options_set(Options* const opts, const char opt);

static void patterns_init(Patterns* const patts);
static void patterns_free(Patterns* const patts);

static void print_help();
static void print_invalid_option();

static void patterns_add_from_string(Patterns* const patts,
                                     const char* const str);
static void patterns_add(Patterns* const patts, const char* const patt);
static void patterns_add_from_file(Patterns* const patts, char* const filename);

static void buffer_file(FILE* file, char* buffer);
// static void process_files(int file_cnt, char* const file_path[],
//                           const Options* const opts);

#endif  // GREP_H
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define OPTIONS_END -1
#define FILE_READ_MODE "r"

static const char SHORT_OPTIONS[] = "AbeEnstTuv";

static const struct option LONG_OPTIONS[] = {{"show-all", 0, NULL, 'A'},
                                             {"number-nonblank", 0, NULL, 'b'},
                                             {"show-ends", 0, NULL, 'E'},
                                             {"number", 0, NULL, 'n'},
                                             {"squeeze-blank", 0, NULL, 's'},
                                             {"show-tabs", 0, NULL, 'T'},
                                             {"show-nonprinting", 0, NULL, 'v'},
                                             {"help", 0, NULL, 0},
                                             {NULL, 0, NULL, 0}};

struct Options {
  bool b;
  bool e;
  bool n;
  bool s;
  bool t;
  bool u;
  bool v;
};
typedef struct Options Options;

static void init_options(int argc, char* const argv[], Options* options);
static void set_option(const char option, Options* options);
static void print_help();
static void print_invalid_option();

static void process_files(int file_count, char* const file_path[],
                          const Options* const options);
static void print_invalid_file(const char* const file_name);
static void print_file(FILE* file, const Options* const options);

int main(int argc, char* argv[]) {
  Options options = {0};
  init_options(argc, argv, &options);
  process_files(argc - optind, argv + optind, &options);
  return EXIT_SUCCESS;
}

static void init_options(int argc, char* const argv[], Options* options) {
  int long_options_index = 0;

  char current_option =
      getopt_long(argc, argv, SHORT_OPTIONS, LONG_OPTIONS, &long_options_index);

  while (current_option != OPTIONS_END) {
    set_option(current_option, options);
    current_option = getopt_long(argc, argv, SHORT_OPTIONS, LONG_OPTIONS,
                                 &long_options_index);
  }
}

static void set_option(const char option, Options* options) {
  switch (option) {
    case 'A':
      options->v = true;
      options->e = true;
      options->t = true;
      break;
    case 'b':
      options->b = true;
      break;
    case 'e':
      options->v = true;
      options->e = true;
      break;
    case 'E':
      options->e = true;
      break;
    case 'n':
      options->n = true;
      break;
    case 's':
      options->s = true;
      break;
    case 't':
      options->v = true;
      options->t = true;
      break;
    case 'T':
      options->t = true;
      break;
    case 'u':
      options->u = true;
      break;
    case 'v':
      options->v = true;
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
  fprintf(stderr,
          "Usage: cat [OPTION]... [FILE]...\nConcatenate FILE(s) to standard "
          "output.\n");

  exit(EXIT_FAILURE);
}

static void print_invalid_option() {
  fprintf(stderr, "Try 'cat --help' for more information.\n");

  exit(EXIT_FAILURE);
}

static void process_files(int file_count, char* const file_path[],
                          const Options* const options) {
  FILE* current_file = NULL;
  while (file_count > 0) {
    current_file = fopen(*file_path, FILE_READ_MODE);
    if (current_file == NULL) {
      print_invalid_file(*file_path);
    } else {
      print_file(current_file, options);
    }
    ++file_path;
    --file_count;
  }
}

static void print_invalid_file(const char* const file_name) {
  fprintf(stderr, "%s: No such file or directory\n", file_name);
}

static void print_file(FILE* file, const Options* const options) {
  if (options) {
    puts("We Are Here!");
  }
  putc(fgetc(file), stdout);
  puts("");
}
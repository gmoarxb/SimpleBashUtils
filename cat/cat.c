#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define GETOPT_OPTIONS_END -1
#define FOPEN_READ_MODE "r"
#define ASCII_DEL 127
#define NONPRINTING_SHIFT 64

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

typedef struct Options {
  bool b;
  bool e;
  bool n;
  bool s;
  bool t;
  bool u;
  bool v;
} Options;

static void init_options(int argc, char* const argv[], Options* const options);
static void set_option(const char option, Options* const options);
static void print_help();
static void print_invalid_option();

static void process_files(int file_count, char* const file_path[],
                          const Options* const options);
static void print_invalid_file(const char* const file_name);
static void print_file(FILE* file, const Options* const options);

static void count_lfd(const char current_symbol, unsigned* const lfd_count);
static void number_line(const char previous_symbol, const char current_symbol,
                        const Options* const options);
static void end_line(const char current_symbol, const Options* const options);
static void print_symbol(const char current_symbol,
                         const Options* const options);
static void print_tab(const char current_symbol, const Options* const options);
static void print_lfd(const char current_symbol);
static void print_cntrl(const char current_symbol,
                        const Options* const options);
static void print_meta(const char current_symbol, const Options* const options);
static void print_plain(const char current_symbol);

int main(int argc, char* argv[]) {
  Options options = {0};
  init_options(argc, argv, &options);
  process_files(argc - optind, argv + optind, &options);
  return EXIT_SUCCESS;
}

static void init_options(int argc, char* const argv[], Options* const options) {
  int long_options_index = 0;
  char current_option =
      getopt_long(argc, argv, SHORT_OPTIONS, LONG_OPTIONS, &long_options_index);
  while (current_option != GETOPT_OPTIONS_END) {
    set_option(current_option, options);
    current_option = getopt_long(argc, argv, SHORT_OPTIONS, LONG_OPTIONS,
                                 &long_options_index);
  }
  if (options->b) {
    options->n = false;
  }
}

static void set_option(const char option, Options* const options) {
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
  fprintf(stdout,
          "Usage: cat [OPTION]... [FILE]...\n"
          "Concatenate FILE(s) to standard output.\n");
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
    current_file = fopen(*file_path, FOPEN_READ_MODE);
    current_file == NULL ? print_invalid_file(*file_path)
                         : print_file(current_file, options);
    fflush(stdout);
    ++file_path;
    --file_count;
  }
}

static void print_invalid_file(const char* const file_name) {
  fprintf(stderr, "cat: %s: No such file or directory\n", file_name);
}

static void print_file(FILE* file, const Options* const options) {
  static unsigned lfd_count = 1;
  static char previous_symbol = '\n';
  char current_symbol = fgetc(file);
  while (!feof(file)) {
    if (options->s) {
      count_lfd(current_symbol, &lfd_count);
    }
    if (current_symbol != '\n' || lfd_count <= 2) {
      number_line(previous_symbol, current_symbol, options);
      end_line(current_symbol, options);
      print_symbol(current_symbol, options);
    }
    previous_symbol = current_symbol;
    current_symbol = fgetc(file);
  }
}

static void count_lfd(const char current_symbol, unsigned* const lfd_count) {
  if (current_symbol == '\n') {
    *lfd_count += 1;
  } else {
    *lfd_count = 0;
  }
}

static void number_line(const char previous_symbol, const char current_symbol,
                        const Options* const options) {
  static unsigned line_count = 0;
  if (options->b && previous_symbol == '\n' && current_symbol != '\n') {
    fprintf(stdout, "%6u\t", ++line_count);
  } else if (options->n && previous_symbol == '\n') {
    fprintf(stdout, "%6u\t", ++line_count);
  }
}

static void end_line(const char current_symbol, const Options* const options) {
  if (options->e && current_symbol == '\n') {
    fputc('$', stdout);
  }
}

static void print_symbol(const char current_symbol,
                         const Options* const options) {
  if (isprint(current_symbol)) {
    print_plain(current_symbol);
  } else if (current_symbol == '\t') {
    print_tab(current_symbol, options);
  } else if (current_symbol == '\n') {
    print_lfd(current_symbol);
  } else if (iscntrl(current_symbol)) {
    print_cntrl(current_symbol, options);
  } else {
    print_meta(current_symbol, options);
  }
}

static void print_tab(const char current_symbol, const Options* const options) {
  if (options->t) {
    print_plain('^');
    print_plain(current_symbol + NONPRINTING_SHIFT);
  } else {
    print_plain(current_symbol);
  }
}

static void print_lfd(const char current_symbol) {
  print_plain(current_symbol);
}

static void print_cntrl(const char current_symbol,
                        const Options* const options) {
  if (options->v) {
    print_plain('^');
    if (current_symbol == ASCII_DEL) {
      print_plain(current_symbol - NONPRINTING_SHIFT);
    } else {
      print_plain(current_symbol + NONPRINTING_SHIFT);
    }
  } else {
    print_plain(current_symbol);
  }
}

static void print_meta(const char current_symbol,
                       const Options* const options) {
  if (options->v) {
    const char meta_symbol = (signed char)current_symbol + ASCII_DEL + 1;
    print_plain('M');
    print_plain('-');
    if (isprint(meta_symbol)) {
      print_plain(meta_symbol);
    } else {
      print_cntrl(meta_symbol, options);
    }
  } else {
    print_plain(current_symbol);
  }
}

static void print_plain(const char current_symbol) {
  fputc(current_symbol, stdout);
}
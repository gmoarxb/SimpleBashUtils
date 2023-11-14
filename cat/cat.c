#include "cat.h"

#include <getopt.h>
#include <stdlib.h>
#include <ctype.h>

#include "../utilities/safe.h"

static const char SHORTOPTS[] = "AbeEnstTuv";

static const struct option LONGOPTS[] = {
    {"show-all", no_argument, NULL, 'A'},
    {"number-nonblank", no_argument, NULL, 'b'},
    {"show-ends", no_argument, NULL, 'E'},
    {"number", no_argument, NULL, 'n'},
    {"squeeze-blank", no_argument, NULL, 's'},
    {"show-tabs", no_argument, NULL, 'T'},
    {"show-nonprinting", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 0},
    {NULL, 0, NULL, 0}};

int main(int argc, char* argv[]) {
  Options opts = {0};
  options_init(&opts, argc, argv);
  process_files(argc - optind, argv + optind, &opts);
  return EXIT_SUCCESS;
}

static void options_init(Options* const opts, int argc, char* const argv[]) {
  int longind = 0;
  char curr_opt = getopt_long(argc, argv, SHORTOPTS, LONGOPTS, &longind);
  while (curr_opt != GETOPT_END) {
    options_set(opts, curr_opt);
    curr_opt = getopt_long(argc, argv, SHORTOPTS, LONGOPTS, &longind);
  }
  if (opts->b) {
    opts->n = false;
  }
}

static void options_set(Options* const opts, const char opt) {
  switch (opt) {
    case 'A':
      opts->v = true;
      opts->e = true;
      opts->t = true;
      break;
    case 'b':
      opts->b = true;
      break;
    case 'e':
      opts->v = true;
      opts->e = true;
      break;
    case 'E':
      opts->e = true;
      break;
    case 'n':
      opts->n = true;
      break;
    case 's':
      opts->s = true;
      break;
    case 't':
      opts->v = true;
      opts->t = true;
      break;
    case 'T':
      opts->t = true;
      break;
    case 'v':
      opts->v = true;
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
          "Usage: cat [OPTION]... [FILE]...\n"
          "Concatenate FILE(s) to standard output.\n");
  exit(EXIT_FAILURE);
}

static void print_invalid_option() {
  fprintf(stderr, "Try 'cat --help' for more information.\n");
  exit(EXIT_FAILURE);
}

static void process_files(int file_count, char* const file_path[],
                          const Options* const opts) {
  for (FILE* curr_file = NULL; file_count--; ++file_path) {
    curr_file = safe_fopen(*file_path, FOPEN_READ);
    cat_file(curr_file, opts);
    fflush(stdout);
    fclose(curr_file);
  }
}

static void cat_file(FILE* file, const Options* const opts) {
  static size_t lfd_count = 1;
  static char prev_sym = '\n';
  char curr_sym = fgetc(file);
  while (!feof(file)) {
    if (opts->s) {
      count_lfd(curr_sym, &lfd_count);
    }
    if (curr_sym != '\n' || lfd_count <= 2) {
      number_line(prev_sym, curr_sym, opts);
      end_line(curr_sym, opts);
      print_symbol(curr_sym, opts);
    }
    prev_sym = curr_sym;
    curr_sym = fgetc(file);
  }
}

static void count_lfd(const char curr_sym, size_t* const lfd_count) {
  if (curr_sym == '\n') {
    *lfd_count += 1;
  } else {
    *lfd_count = 0;
  }
}

static void number_line(const char prev_sym, const char curr_sym,
                        const Options* const opts) {
  static size_t line_count = 0;
  if (opts->b && prev_sym == '\n' && curr_sym != '\n') {
    fprintf(stdout, "%6zu\t", ++line_count);
  } else if (opts->n && prev_sym == '\n') {
    fprintf(stdout, "%6zu\t", ++line_count);
  }
}

static void end_line(const char curr_sym, const Options* const opts) {
  if (opts->e && curr_sym == '\n') {
    fputc('$', stdout);
  }
}

static void print_symbol(const char curr_sym, const Options* const opts) {
  if (isprint(curr_sym)) {
    print_plain(curr_sym);
  } else if (curr_sym == '\t') {
    print_tab(opts);
  } else if (curr_sym == '\n') {
    print_lfd();
  } else if (iscntrl(curr_sym)) {
    print_cntrl(curr_sym, opts);
  } else {
    print_meta(curr_sym, opts);
  }
}

static void print_tab(const Options* const opts) {
  if (opts->t) {
    print_plain('^');
    print_plain('\t' + NONPRINT_SHIFT);
  } else {
    print_plain('\t');
  }
}

static void print_lfd() { print_plain('\n'); }

static void print_cntrl(const char curr_sym, const Options* const opts) {
  if (opts->v) {
    print_plain('^');
    if (curr_sym == ASCII_DEL) {
      print_plain(curr_sym - NONPRINT_SHIFT);
    } else {
      print_plain(curr_sym + NONPRINT_SHIFT);
    }
  } else {
    print_plain(curr_sym);
  }
}

static void print_meta(const char curr_sym, const Options* const opts) {
  if (opts->v) {
    const char meta_symbol = (signed char)curr_sym + ASCII_DEL + 1;
    print_plain('M');
    print_plain('-');
    if (isprint(meta_symbol)) {
      print_plain(meta_symbol);
    } else {
      print_cntrl(meta_symbol, opts);
    }
  } else {
    print_plain(curr_sym);
  }
}

static void print_plain(const char curr_sym) { fputc(curr_sym, stdout); }
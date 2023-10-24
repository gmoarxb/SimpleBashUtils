#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define OPTIONS_END -1

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

static void set_option(const char option, Options *options);
static void print_help();
static void print_invalid_option();

int main(int argc, char *argv[]) {
  Options options = {0};

  int long_options_index = 0;

  char current_option =
      getopt_long(argc, argv, SHORT_OPTIONS, LONG_OPTIONS, &long_options_index);

  while (current_option != OPTIONS_END) {
    set_option(current_option, &options);
    current_option = getopt_long(argc, argv, SHORT_OPTIONS, LONG_OPTIONS,
                                 &long_options_index);
  }

  printf("b - %d\ne - %d\nn - %d\ns - %d\nt - %d\nu - %d\nv - %d\n", options.b,
         options.e, options.n, options.s, options.t, options.u, options.v);

  while (optind < argc) {
    puts(argv[optind++]);
  }

  return EXIT_SUCCESS;
}

static void set_option(const char option, Options *options) {
  switch (option) {
    case 'A':
      options->v = true;
      options->e = true;
      options->t = true;
      break;
    case 'b':
      options->b = true;
      options->n = false;
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
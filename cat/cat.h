#ifndef CAT_H
#define CAT_H

#include <stdbool.h>
#include <stdio.h>

#define GETOPT_END -1
#define FOPEN_READ "r"
#define ASCII_DEL 127
#define NONPRINT_SHIFT 64

typedef struct Options {
  bool b;
  bool e;
  bool n;
  bool s;
  bool t;
  bool u;
  bool v;
} Options;

static void options_init(Options* const opts, int argc, char* const argv[]);
static void options_set(Options* const opts, const char opt);
static void print_help();
static void print_invalid_option();

static void process_files(int file_count, char* const file_path[],
                          const Options* const opts);
static void print_file(FILE* file, const Options* const opts);

static void count_lfd(const char curr_sym, size_t* const lfd_count);
static void number_line(const char prev_sym, const char curr_sym,
                        const Options* const opts);
static void end_line(const char curr_sym, const Options* const opts);

static void print_symbol(const char curr_sym, const Options* const opts);
static void print_plain(const char curr_sym);
static void print_tab(const Options* const opts);
static void print_lfd();
static void print_cntrl(const char curr_sym, const Options* const opts);
static void print_meta(const char curr_sym, const Options* const opts);

#endif  // CAT_H
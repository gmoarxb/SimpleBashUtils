#include "grep.h"

#include <ctype.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "../utilities/safe.h"

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

int main(int argc, char *argv[]) {
  Options opts = {0};
  options_init(&opts, argc, argv);
  process_files(argc - optind, argv + optind, &opts);
  options_free(&opts);
  return 0;
}

static void options_init(Options *const opts, int argc, char *const argv[]) {
  patterns_init(&opts->patts);
  int longind = 0;
  char curr_opt = getopt_long(argc, argv, SHORTOPTS, LONGOPTS, &longind);
  while (curr_opt != GETOPT_END) {
    options_set(opts, curr_opt);
    curr_opt = getopt_long(argc, argv, SHORTOPTS, LONGOPTS, &longind);
  }
  if (!opts->e && !opts->f) {
    patterns_add_from_string(&opts->patts, argv[optind++]);
  }
  patterns_compile_to_regex(opts);
  opts->file_count = argc - optind;
}

static void patterns_compile_to_regex(Options *const opts) {
  Patterns *patts = &opts->patts;
  int reg_icase = opts->i ? REG_ICASE : 0;
  for (size_t i = 0; i < patts->cur_size; ++i) {
    regcomp(&patts->reg_data[i], patts->data[i], reg_icase);
  }
}

static void options_free(Options *const opts) { patterns_free(&opts->patts); }

static void patterns_init(Patterns *const patts) {
  patts->cur_size = 0;
  patts->max_size = PATTERNS_INIT;
  patts->data = safe_malloc(sizeof(char *) * patts->max_size);
  patts->reg_data = safe_malloc(sizeof(regex_t) * patts->max_size);
}

static void patterns_free(Patterns *const patts) {
  for (size_t i = 0; i < patts->cur_size; ++i) {
    free(patts->data[i]);
    regfree(&patts->reg_data[i]);
  }
  free(patts->data);
  free(patts->reg_data);
}

static void options_set(Options *const opts, const char opt) {
  switch (opt) {
  case 'e':
    opts->e = true;
    patterns_add_from_string(&opts->patts, optarg);
    break;
  case 'f':
    opts->f = true;
    patterns_add_from_file(&opts->patts, optarg);
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
  fprintf(stdout, "Usage: grep [OPTION]... PATTERNS [FILE]...\n"
                  "Search for PATTERNS in each FILE.\n");
  exit(EXIT_FAILURE);
}

static void print_invalid_option() {
  fprintf(stderr, "Try 'grep --help' for more information.\n");
  exit(EXIT_FAILURE);
}

static void patterns_add_from_string(Patterns *const patts,
                                     const char *const str) {
  char *temp_patts = safe_malloc(sizeof(char) * strlen(str) + sizeof(char));
  strcpy(temp_patts, str);
  char *token = strtok(temp_patts, "\n");
  while (token != NULL) {
    patterns_add(patts, token);
    token = strtok(NULL, "\n");
  }
  free(temp_patts);
}

static void patterns_add(Patterns *const patts, const char *const patt) {
  if (patts->cur_size == patts->max_size) {
    patts->max_size += PATTERNS_ADD;
    patts->data = safe_realloc(patts->data, patts->max_size * sizeof(char *));
  }
  patts->data[patts->cur_size] =
      safe_malloc(sizeof(char) * strlen(patt) + sizeof(char));
  strcpy(patts->data[patts->cur_size++], patt);
}

static void patterns_add_from_file(Patterns *const patts,
                                   char *const filename) {
  FILE *file = safe_fopen(filename, FOPEN_READ);
  char *buffer = safe_malloc(sizeof(char) * BUFFER_INIT);
  buffer_file(file, buffer);
  patterns_add_from_string(patts, buffer);
  free(buffer);
  fclose(file);
}

static void buffer_file(FILE *file, char *buffer) {
  size_t size = 0;
  size_t max_size = BUFFER_INIT;
  char symbol = fgetc(file);
  while (!feof(file)) {
    buffer[size++] = symbol;
    symbol = fgetc(file);
    if (size == max_size) {
      max_size *= BUFFER_MULT;
      buffer = safe_realloc(buffer, max_size * sizeof(char));
    }
  }
  buffer[size] = '\0';
}

static void process_files(int file_count, char *const file_path[],
                          const Options *const opts) {
  for (FILE *curr_file = NULL; file_count--; ++file_path) {
    curr_file = fopen(*file_path, FOPEN_READ);
    if (curr_file != NULL) {
      route_file_greping(curr_file, *file_path, opts);
      fflush(stdout);
      fclose(curr_file);
    } else if (!opts->s) {
      fprintf(stderr, "%s: No such file or directory\n", *file_path);
    }
  }
}

static void route_file_greping(FILE *file, const char *filename,
                               const Options *const opts) {
  if (opts->l) {
    grep_files_with_matches(file, filename, opts);
  } else if (opts->c) {
    grep_match_count(file, filename, opts);
  } else {
    grep_lines_with_matches(file, filename, opts);
  }
  // else if (opts->o) {
  //   grep_only_matching(file, filename, opts);
  // } else {
}

static void grep_files_with_matches(FILE *file, const char *filename,
                                    const Options *const opts) {
  char *buffer = safe_malloc(sizeof(char) * BUFFER_INIT);
  size_t buffer_size = BUFFER_INIT;
  while (getline(&buffer, &buffer_size, file) != EOF) {
    if (is_match(buffer, opts)) {
      fprintf(stdout, "%s\n", filename);
      break;
    }
  }
  free(buffer);
}

static bool is_match(char *line, const Options *const opts) {
  const Patterns *const patts = &opts->patts;
  bool result = false;
  for (size_t i = 0; i < patts->cur_size; ++i) {
    if (regexec(&patts->reg_data[i], line, 0, NULL, 0) == 0) {
      result = true;
    }
  }
  if (opts->v) {
    result = !result;
  }
  return result;
}

static void grep_match_count(FILE *file, const char *filename,
                             const Options *const opts) {
  size_t match_count = 0;
  char *buffer = safe_malloc(sizeof(char) * BUFFER_INIT);
  size_t buffer_size = BUFFER_INIT;
  while (getline(&buffer, &buffer_size, file) != EOF) {
    if (is_match(buffer, opts)) {
      ++match_count;
    }
  }
  if (opts->file_count > 1 && !opts->h) {
    fprintf(stdout, "%s:", filename);
  }
  fprintf(stdout, "%zu\n", match_count);
  free(buffer);
}

static void grep_lines_with_matches(FILE *file, const char *filename,
                                    const Options *const opts) {
  char *line = safe_malloc(sizeof(char) * BUFFER_INIT);
  size_t line_size = BUFFER_INIT;
  while (getline(&line, &line_size, file) != EOF) {
    if (is_match(line, opts)) {
      if (opts->file_count > 1 && !opts->h) {
        fprintf(stdout, "%s:", filename);
      }
      fprintf(stdout, "%s", line);
    }
  }
  free(line);
}
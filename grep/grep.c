#include "grep.h"

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

int main(int argc, char* argv[]) {
  Options opts = {0};
  options_init(&opts, argc, argv);
  process_files(argc - optind, argv + optind, &opts);
  options_free(&opts);
  return 0;
}

static void options_init(Options* const opts, int argc, char* const argv[]) {
  patterns_init(&opts->patts);
  int longind = 0;
  char curr_opt = getopt_long(argc, argv, SHORTOPTS, LONGOPTS, &longind);
  while (curr_opt != GETOPT_END) {
    options_set(opts, curr_opt);
    curr_opt = getopt_long(argc, argv, SHORTOPTS, LONGOPTS, &longind);
  }
}

static void options_free(Options* const opts) { patterns_free(&opts->patts); }

static void patterns_init(Patterns* const patts) {
  patts->cur_size = 0;
  patts->max_size = PATTERNS_INIT;
  patts->data = safe_malloc(sizeof(char*) * patts->max_size);
}

static void patterns_free(Patterns* const patts) {
  for (size_t i = 0; i < patts->cur_size; ++i) {
    free(patts->data[i]);
  }
  free(patts->data);
}

static void options_set(Options* const opts, const char opt) {
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
  fprintf(stdout,
          "Usage: grep [OPTION]... PATTERNS [FILE]...\n"
          "Search for PATTERNS in each FILE.\n");
  exit(EXIT_FAILURE);
}

static void print_invalid_option() {
  fprintf(stderr, "Try 'grep --help' for more information.\n");
  exit(EXIT_FAILURE);
}

static void patterns_add_from_string(Patterns* const patts,
                                     const char* const str) {
  char* temp_patts = safe_malloc(sizeof(char) * strlen(str) + sizeof(char));
  strcpy(temp_patts, str);
  char* token = strtok(temp_patts, "\n");
  while (token != NULL) {
    patterns_add(patts, token);
    token = strtok(NULL, "\n");
  }
  free(temp_patts);
}

static void patterns_add(Patterns* const patts, const char* const patt) {
  if (patts->cur_size == patts->max_size) {
    patts->max_size += PATTERNS_ADD;
    patts->data = safe_realloc(patts->data, patts->max_size * sizeof(char*));
  }
  patts->data[patts->cur_size] =
      safe_malloc(sizeof(char) * strlen(patt) + sizeof(char));
  strcpy(patts->data[patts->cur_size++], patt);
}

static void patterns_add_from_file(Patterns* const patts,
                                   char* const filename) {
  FILE* file = safe_fopen(filename, FOPEN_READ);
  char* buffer = safe_malloc(sizeof(char) * BUFFER_INIT);
  buffer_file(file, buffer);
  patterns_add_from_string(patts, buffer);
  free(buffer);
  fclose(file);
}

static void buffer_file(FILE* file, char* buffer) {
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

static void process_files(int file_count, char* const file_path[],
                          const Options* const opts) {
  for (FILE* curr_file = NULL; file_count--; ++file_path) {
    curr_file = fopen(*file_path, FOPEN_READ);
    if (curr_file != NULL) {
      grep_file(curr_file, *file_path, opts);
      fflush(stdout);
      fclose(curr_file);
    } else if (!opts->s) {
      fprintf(stderr, "%s: No such file or directory\n", *file_path);
    }
  }
}

static void grep_file(FILE* file, const char* filename, const Options* const opts) {
  char* line = NULL;
  size_t line_size = 0;
  while (getline(&line, &line_size, file) != -1) {
    for (size_t i = 0; i < opts->patts.cur_size; ++i) {
      bool found = find_matches(line, opts);
      if (opts->v) {
        found = !found;
      }
      if (found) {
        if (!opts->h) {
          fputs(filename, stdout);
          fputc(':', stdout);
        }
        fputs(line, stdout);
      }
    }
  }
  free(line);
}

static bool find_matches(const char* const line, const Options* const opts) {
  bool found = false;
  char* temp_line = safe_malloc(sizeof(char) * strlen(line) + sizeof(char));
  for (size_t i = 0; i < strlen(line); ++i) {
    temp_line[i] = opts->i ? tolower(line[i]) : line[i]; 
  }
  temp_line[strlen(line)] = '\0';
  for (size_t i = 0; i < opts->patts.cur_size && !found; ++i) {
    char* temp_pattern = safe_malloc(sizeof(char) * strlen(opts->patts.data[i]) + sizeof(char));
    for (size_t j = 0; j < strlen(opts->patts.data[i]); ++j) {
      temp_pattern[j] = opts->i ? tolower(opts->patts.data[i][j]) : opts->patts.data[i][j];
    }
    temp_pattern[strlen(opts->patts.data[i])] = '\0';
    if (strstr(temp_line, temp_pattern) != NULL) {
      found = true;
    }
    free(temp_pattern);
  }
  free(temp_line);
  return found;
}
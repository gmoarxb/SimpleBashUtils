#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
  srand(time(NULL));

  FILE* ascii_symbols = fopen("./ascii.txt", "w");

  putc('\n', ascii_symbols);
  putc('\n', ascii_symbols);
  fputs("___", ascii_symbols);

  for (char c = 0; ; ++c) {
    putc(c, ascii_symbols);
    putc('\n', ascii_symbols);
    if (c == 127) break;
  }

  putc('\n', ascii_symbols);
  putc('\n', ascii_symbols);
  fputs("___", ascii_symbols);

  for (int i = 0; i < 1280; ++i) {
    putc((char)(rand() % 128), ascii_symbols);
  }

  return 0;
}
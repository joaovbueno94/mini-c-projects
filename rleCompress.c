// Copyright 2026 João Vinícius Bueno

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void compress(const char *filename) {
  FILE *infile = fopen(filename, "r");
  if (infile == NULL) {
    perror("[error] fopen");
    exit(errno);
  }

  char outfilename[255];
  snprintf(outfilename, sizeof(outfilename), "%s-compressed", filename);
  FILE *outfile = fopen(outfilename, "w");
  if (outfile == NULL) {
    perror("[error] fopen");
    exit(errno);
  }

  int seen_char = fgetc(infile);
  int next_char;
  int repeat_counter = 1;

  while ((next_char = fgetc(infile)) != EOF) {
    if (next_char == seen_char) {
      ++repeat_counter;
      if (repeat_counter >= 255) {
        fputc(seen_char, outfile);
        fputc(repeat_counter, outfile);
        repeat_counter = 0;
      }
    } else {
      fputc(seen_char, outfile);
      fputc(repeat_counter, outfile);
      repeat_counter = 1;
      seen_char = next_char;
    }
  }

  fclose(infile);
  fclose(outfile);
}

void decompress(const char *filename) {
  FILE *infile = fopen(filename, "r");
  if (infile == NULL) {
    perror("[error] fopen");
    exit(errno);
  }

  char outfilename[255];
  snprintf(outfilename, sizeof(outfilename), "%s-decompressed", filename);
  FILE *outfile = fopen(outfilename, "w");
  if (outfile == NULL) {
    perror("[error] fopen");
    exit(errno);
  }

  int c, count;
  while (((c = fgetc(infile)) != EOF) | ((count = fgetc(infile)) != EOF)) {
    for (int i = 1; i <= count; ++i) {
      fputc(c, outfile);
    }
  }

  fclose(infile);
  fclose(outfile);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "[error] Usage: %s [compress|decompress] <filename>\n",
            argv[0]);
    return 1;
  }

  if (!strcmp(argv[1], "compress")) {
    compress(argv[2]);
  } else if (!strcmp(argv[1], "decompress")) {
    decompress(argv[2]);
  } else {
    fprintf(stderr, "[error] Usage: %s [compress|decompress] <filename>\n",
            argv[0]);
    return 1;
  }

  return 0;
}

// Copyright 2026 João Vinícius Bueno

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

void split(const char *filename, const size_t numParts) {
  FILE *infile = fopen(filename, "r");
  if (infile == NULL) {
    perror("[error] fopen");
    exit(errno);
  }

  struct stat sb;
  if (stat(filename, &sb) == -1) {
    perror("[error] stat");
    exit(errno);
  }

  size_t partSize = sb.st_size / numParts;
  size_t lastPartSize = sb.st_size - (partSize * (numParts - 1));

  char *buffer = malloc(lastPartSize);
  if (buffer == NULL) {
    perror("[error] malloc");
    exit(errno);
  }

  char outpath[255];
  clearerr(infile);

  for (size_t i = 1; i <= numParts - 1; ++i) {
    fread(buffer, 1, partSize, infile);
    if (ferror(infile)) {
      fprintf(stderr, "[error] reading file failed\n");
      exit(1);
    }

    snprintf(outpath, sizeof(outpath), "%s-part-%ld", filename, i);
    FILE *outfile = fopen(outpath, "w");
    if (outfile == NULL) {
      perror("[error] fopen");
      exit(errno);
    }

    fwrite(buffer, 1, partSize, outfile);
    if (ferror(outfile)) {
      fprintf(stderr, "[error] writting file failed\n");
      exit(1);
    }

    fclose(outfile);
  }

  fread(buffer, 1, lastPartSize, infile);
  if (ferror(infile)) {
    fprintf(stderr, "[error] reading file failed\n");
    exit(1);
  }
  if (feof(infile)) {
    fprintf(stderr, "[error] bytes left without reading\n");
    exit(1);
  }

  snprintf(outpath, sizeof(outpath), "%s-part-%ld", filename, numParts);
  FILE *outfile = fopen(outpath, "w");
  if (outfile == NULL) {
    perror("[error] fopen");
    exit(errno);
  }

  fwrite(buffer, 1, lastPartSize, outfile);
  if (ferror(outfile)) {
    fprintf(stderr, "[error] writting file failed\n");
    exit(1);
  }

  fclose(infile);
  fclose(outfile);
  free(buffer);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "[error] Usage: %s <filename> <num-parts>\n", argv[0]);
    return 1;
  }

  split(argv[1], atol(argv[2]));

  return 0;
}

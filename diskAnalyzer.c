// Copyright 2026 João Vinícius Bueno

#define _GNU_SOURCE
#define _XOPEN_SOURCE 500

#include <ftw.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_FILENAME_SIZE 255
#define INITIAL_ARRAY_SIZE 1

typedef struct {
  char filename[MAX_FILENAME_SIZE];
  int64_t filesize;
} fileEntry;

fileEntry *files = NULL;
size_t currentIndex = 0;
size_t currentArraySize = INITIAL_ARRAY_SIZE;

void createArray(void) {
  files = calloc(INITIAL_ARRAY_SIZE, sizeof(fileEntry));
  if (files == NULL) {
    perror("[error] calloc");
    free(files);
    exit(1);
  }
}

int fillArray(const char *fpath, const struct stat *sb, int typeflag,
              struct FTW *ftwbuf) {
  if (typeflag == FTW_SL || typeflag == FTW_SLN) {
    return FTW_CONTINUE;
  }

  if (currentIndex >= currentArraySize) {
    files = reallocarray(files, 2 * currentArraySize, sizeof(fileEntry));
    if (files == NULL) {
      perror("[error] reallocarray");
      free(files);
      exit(1);
    }
    currentArraySize *= 2;
  }

  snprintf(files[currentIndex].filename, MAX_FILENAME_SIZE, "%s", fpath);
  files[currentIndex].filesize = sb->st_size;
  ++currentIndex;

  if (ftwbuf->level > 0) {
    return FTW_SKIP_SUBTREE;
  }
  return FTW_CONTINUE;
}

void walk(const char *dir) {
  if (nftw(dir, fillArray, 1, FTW_ACTIONRETVAL) == -1) {
    perror("[error] nftw");
    free(files);
    exit(1);
  }
}

int sizeSort(const void *first, const void *second) {
  const fileEntry *fPtr = first;
  const fileEntry *sPtr = second;
  if (fPtr->filesize > sPtr->filesize) {
    return -1;
  } else if (fPtr->filesize < sPtr->filesize) {
    return 1;
  }
  return 0;
}

void printFiles(void) {
  qsort(files, currentIndex, sizeof(fileEntry), sizeSort);

  puts("Disk Analyzer:\n----------------");
  printf("%-12s%s\n", "Size (B)", "File");
  puts("----------------");
  for (size_t i = 0; i < currentIndex; ++i) {
    size_t blocks = 75 * files[i].filesize / files[0].filesize;
    printf("%-12ld%s\n", files[i].filesize, files[i].filename);
    for (size_t j = 1; j <= blocks; ++j) {
      printf("%c%c%c", 0xe2, 0x96, 0x88);
    }
    puts("");
  }
}

int main(int argc, char *argv[]) {
  createArray();

  if (argc > 2) {
    fputs("[error] args: too many arguments\n", stderr);
    free(files);
    return 1;
  } else if (argc == 2) {
    walk(argv[1]);
  } else if (isatty(fileno(stdin))) {
    walk(".");
  } else {
    fputs("[error] isatty: not supported\n", stderr);
    free(files);
    return 1;
  }

  printFiles();
  free(files);

  return 0;
}

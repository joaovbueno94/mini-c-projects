// Copyright 2026 João Vinícius Bueno

#define _XOPEN_SOURCE 500
#define _GNU_SOURCE

#include <ftw.h>
#include <stdio.h>

int printName(const char *fpath, const struct stat *sb, int typeflag,
              struct FTW *ftwbuf) {
  if (typeflag == FTW_F && (sb->st_mode & S_IXUSR)) {
    printf("\e[1;32m%s\e[0m  ", fpath);
  } else if (typeflag == FTW_D || typeflag == FTW_DNR) {
    printf("\e[1;34m%s\e[0m  ", fpath);
  } else if (typeflag == FTW_SL || typeflag == FTW_SLN) {
    printf("\e[1;36m%s\e[0m  ", fpath);
  } else {
    printf("\e[0m%s  ", fpath);
  }

  if (ftwbuf->level > 0) {
    return FTW_SKIP_SUBTREE;
  }
  return FTW_CONTINUE;
}

int printDirContents(const char *dir) {
  return nftw(dir, printName, 1, FTW_ACTIONRETVAL);
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    // List contents of the current directory
    printDirContents(".");
  } else if (argc == 2) {
    // List contents of given directory
    if (printDirContents(argv[1]) == -1) {
      printf("%s: unable to access '%s': no such file or directory\n", argv[0],
             argv[1]);
      return argc;
    }
  } else {
    printf("Too many arguments given. Usage: %s <dir>\n", argv[0]);
    return argc;
  }
  puts("");

  return 0;
}

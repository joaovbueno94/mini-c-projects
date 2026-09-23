// Copyright 2026 João Vinícius Bueno

/*
 * This code allows copying files, like cp command.
 * To copy a single file, the first argument must be the source file path
 * and the second argument must be the target file path.
 * (EX: ./mycp SOURCE DEST)
 * To copy two or more files at once, the last argument must be a target
 * directory and the previous arguments must be the source files paths.
 * (EX: ./mycp SOURCE... DIR).
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // dealing with missing operands
  if (argc == 1) {
    printf("%s: missing file operand\n", argv[0]);
    return argc;
  } else if (argc == 2) {
    printf("%s: missing destination file operand after '%s'\n", argv[0],
           argv[1]);
    return argc;
  }

  struct stat sourceFileStats;
  int sourceFileFD;
  int targetFileFD;

  // single file copy
  if (argc == 3) {
    stat(argv[1], &sourceFileStats);
    sourceFileFD = open(argv[1], O_RDONLY, 0);
    if (sourceFileFD < 0) {
      printf("%s: cannot open '%s'. No such file or directory\n", argv[0],
             argv[1]);
      return argc;
    }

    targetFileFD = creat(argv[2], sourceFileStats.st_mode);
    if (targetFileFD < 0) {
      printf("%s: cannot open '%s'\n", argv[0], argv[2]);
      return argc;
    }

    char fileBuffer[sourceFileStats.st_size]; // NOLINT
    read(sourceFileFD, fileBuffer, sourceFileStats.st_size);
    write(targetFileFD, fileBuffer, sourceFileStats.st_size);
  } else {
    // multiple file copy
    // last argument must be a directory
    struct stat targetDirStats;
    stat(argv[argc - 1], &targetDirStats);
    if (!S_ISDIR(targetDirStats.st_mode)) {
      printf("%s: target '%s': no such file or directory\n", argv[0],
             argv[argc - 1]);
      return argc;
    }

    int targetDirFD = open(argv[argc - 1], O_RDONLY, 0);

    for (int i = 1; i < argc - 1; ++i) {
      stat(argv[i], &sourceFileStats);
      sourceFileFD = open(argv[i], O_RDONLY, 0);
      if (sourceFileFD < 0) {
        printf("%s: cannot open '%s'. No such file or directory\n", argv[0],
               argv[i]);
        return argc;
      }

      targetFileFD = openat(targetDirFD, argv[i], O_CREAT | O_WRONLY | O_TRUNC,
                            sourceFileStats.st_mode);
      if (targetFileFD < 0) {
        printf("%s: cannot open '%s'\n", argv[0], argv[i]);
        return argc;
      }

      char fileBuffer[sourceFileStats.st_size]; // NOLINT
      read(sourceFileFD, fileBuffer, sourceFileStats.st_size);
      write(targetFileFD, fileBuffer, sourceFileStats.st_size);
    }

    close(targetDirFD);
  }

  close(sourceFileFD);
  close(targetFileFD);

  return 0;
}

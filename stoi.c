// Copyright 2026 João Vinícius Bueno

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

int stoi(const char *s) {
  int currentNum = 0;
  bool isNegative = false;

  if (*s == '-') {
    ++s;
    isNegative = true;
  } else if (*s == '+') {
    ++s;
  }

  while (isdigit(*s)) {
    currentNum = (currentNum * 10) + *s - '0';
    ++s;
  }

  return isNegative ? -currentNum : currentNum;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <number>\n", argv[0]);
    return argc;
  }

  int number = stoi(argv[1]);
  printf("Converted number = %d\n", number);

  return 0;
}

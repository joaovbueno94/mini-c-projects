// Copyright 2026 João Vinícius Bueno

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEBUG(fmt, ...)                                                        \
  if (gDebug) {                                                                \
    fprintf(stderr, "[debug] [%s() line %d] ", __func__, __LINE__);            \
    fprintf(stderr, fmt, ##__VA_ARGS__);                                       \
  }

uint32_t s[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
                4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

uint32_t K[] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
    0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
    0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
    0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
    0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
    0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

uint32_t a0 = 0x67452301;
uint32_t b0 = 0xefcdab89;
uint32_t c0 = 0x98badcfe;
uint32_t d0 = 0x10325476;

bool gDebug = false;
uint64_t gByteCount = 0;
uint8_t gBlock[64] = {0};
ssize_t gBlockIdx = 0;

void process_block(void) {
  DEBUG("processing block\n");

  // initialize hash value for this block
  uint32_t A = a0, B = b0, C = c0, D = d0;
  uint32_t M[16];
  uint32_t F, g;
  ssize_t i, j;

  // break block into sixteen 32-bit words M[j], 0 <= j <= 15
  for (i = 0; i < 16; ++i) {
    j = i * 4;
    M[i] = gBlock[j] | gBlock[j + 1] << 8 | gBlock[j + 2] << 16 |
           gBlock[j + 3] << 24;
  }

  // main loop
  for (i = 0; i < 64; ++i) {
    if (i < 16) {
      F = (B & C) | ((~B) & D);
      g = i;
    } else if (i < 32) {
      F = (D & B) | ((~D) & C);
      g = (5 * i + 1) % 16;
    } else if (i < 48) {
      F = B ^ C ^ D;
      g = (3 * i + 5) % 16;
    } else {
      F = C ^ (B | (~D));
      g = (7 * i) % 16;
    }

    F += A + K[i] + M[g];
    A = D;
    D = C;
    C = B;
    B += (F << s[i] | (F >> (32 - s[i])));
  }

  a0 += A;
  b0 += B;
  c0 += C;
  d0 += D;
}

static inline void process_byte(uint8_t byte) {
  DEBUG("processing byte: %u\n", byte);

  // store the byte
  gBlock[gBlockIdx++] = byte;

  // check if the block is full and ready for processing
  if (gBlockIdx == 64) {
    process_block();
    gBlockIdx = 0;
  }
}

// return 0 on success, -1 on failure
int process_input(int fd) {
  uint8_t buffer[64 * 4096];
  uint8_t byte;
  ssize_t nBytes, i;

  while (true) {
    // read data from the file descriptor
    nBytes = read(fd, buffer, sizeof(buffer));

    if (nBytes == -1) {
      // error
      switch (errno) {
      case EINTR:
        continue;
      default:
        perror("[error] read");
        return -1;
      }
    } else if (nBytes == 0) {
      // done reading / EOF
      break;
    }

    // positive-number: reading is complete
    // loop data byte-by-byte
    for (i = 0; i < nBytes; ++i) {
      DEBUG("need to process byte %u\n", buffer[i]);
      process_byte(buffer[i]);
      ++gByteCount;
    }
  }

  // the entire message was read, now it's necessary to finalize it by padding
  // it and appending the length
  // pad it with 1 and then the necessary number of 0s
  process_byte(0x80);
  while (gBlockIdx != 56) {
    process_byte(0);
  }

  // append original length in bits mod 2^64 to message
  // turn the byte length into 2 numbers - high bits and low bits
  uint32_t lowBits = gByteCount << 3;
  uint32_t highBits = (gByteCount >> (32 - 3));

  // encode the full 64bit int (bits length) as little endian
  for (i = 0; i < 4; ++i) {
    byte = lowBits >> (i * 8);
    DEBUG("lowBits[%ld]=%u\n", i, byte);
    process_byte(byte);
  }
  for (i = 0; i < 4; ++i) {
    byte = highBits >> (i * 8);
    DEBUG("highBits[%ld]=%u\n", i, byte);
    process_byte(byte);
  }

  if (gBlockIdx != 0) {
    fputs("[error] array: bad block index", stderr);
    return -1;
  }

  return 0;
}

static inline void print_hash(void) {
  uint32_t words[] = {a0, b0, c0, d0};
  uint8_t byte;

  for (ssize_t i = 0; i < 4; ++i) {
    for (ssize_t j = 0; j < 4; ++j) {
      byte = words[i] >> (j * 8) & 0xff;
      printf("%02x", byte);
    }
  }
  puts("");
}

int main(int argc, char *argv[]) {
  gDebug = getenv("DEBUG") != NULL;
  int fd = fileno(stdin);

  if (argc > 2) {
    fputs("[error] args: too many arguments\n", stderr);
    return 1;
  } else if (argc == 2) {
    // read from the file given as an argument
    DEBUG("opening file: %s\n", argv[1]);
    fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
      perror("[error] open");
      return 1;
    }
  } else if (isatty(fd)) {
    fputs("[error] isatty: not supported", stderr);
    return 1;
  }

  // fd is ready for reading
  process_input(fd);
  close(fd);
  print_hash();

  return 0;
}

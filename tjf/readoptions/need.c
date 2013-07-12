#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "need.h"

int
needopen(const char* filename, int bits)
{
  int fd = open(filename, bits);
  if(fd == -1) {
    perror("opening data file");
    exit(EXIT_FAILURE);
  }
  return fd;
}

int
needopenmode(const char* filename, int bits, mode_t mode)
{
  int fd = open(filename, bits, mode);
  if(fd == -1) {
    perror("opening data file");
    exit(EXIT_FAILURE);
  }
  return fd;
}

void
needclose(int fd)
{
  if(close(fd) != 0) {
    perror("closing fd");
    exit(EXIT_FAILURE);
  }
}

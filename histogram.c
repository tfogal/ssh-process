#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "histogram.h"
#include "tjf/readoptions/need.h"

static const size_t HIST_SIZE = 65535;

struct histoprocessor {
  func_init* init;
  func_process* run;
  func_finished* fini;
  uint64_t* histo;
  bool is_signed;
  size_t bpc;
};

static void
init(void* self, const bool is_signed, const size_t bpc)
{
  struct histoprocessor* hproc = (struct histoprocessor*) self;
  hproc->is_signed = is_signed;
  hproc->bpc = bpc;
}

#define INNER_HISTO_BIAS(type, bias) \
  do { \
    const type* data = (const type*)d; \
    for(size_t i=0; i < nelems; ++i) { \
      const uint16_t idx = data[i] + bias; \
      ++this->histo[idx]; \
    } \
  } while(0)

static void histo(void* self, const void* d, const size_t nelems)
{
  struct histoprocessor* this = (struct histoprocessor*) self;
  assert(this->histo != NULL);
  switch(this->bpc) {
    case 1:
      if(this->is_signed) {
        INNER_HISTO_BIAS(int8_t, -SCHAR_MIN);
        assert(SCHAR_MIN < 0);
      } else {
        INNER_HISTO_BIAS(uint8_t, 0);
      }
      break;
    case 2:
      if(this->is_signed) {
        INNER_HISTO_BIAS(int16_t, -SHRT_MIN);
        assert(SHRT_MAX < 0);
      } else {
        INNER_HISTO_BIAS(uint16_t, 0);
      }
      break;
    case 4: /* fall-through */
    case 8: /* fall-through */
    default: abort();
  }
}

static void writehisto(void* self) {
  struct histoprocessor* this = (struct histoprocessor*) self;
  assert(this->histo != NULL);

  const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP;
  int fd = needopenmode(".histogram-u64", O_CREAT | O_TRUNC | O_WRONLY, mode);
  ssize_t bytes = write(fd, this->histo, sizeof(uint64_t)*HIST_SIZE);
  if(bytes != sizeof(uint64_t)*HIST_SIZE) {
    perror("short write");
    exit(EXIT_FAILURE);
  }
  needclose(fd);
}

struct processor* histogram() {
  struct histoprocessor* hist = calloc(sizeof(struct histoprocessor), 1);
  hist->init = init;
  hist->run = histo;
  hist->fini = writehisto;
  hist->histo = calloc(sizeof(uint64_t), HIST_SIZE);
  return (struct processor*)hist;
}

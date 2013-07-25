#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mcubes.h"

struct mcubes {
  func_init* init;
  func_process* run;
  func_finished* fini;
  bool is_signed;
  size_t bpc;
  FILE* triangles;
  size_t dims[3];
};

static void
init(void* self, const bool is_signed, const size_t bpc, const size_t d[3])
{
  struct mcubes* mc = (struct mcubes*) self;
  mc->is_signed = is_signed;
  mc->bpc = bpc;
  mc->triangles = fopen(".triangles", "wb");
  memcpy(mc->dims, d, sizeof(size_t)*3);
}

static void
march(void* self, const void* d, const size_t nelems)
{
  struct mcubes* this = (struct mcubes*) self;

  switch(this->bpc) {
    case 2:
      if(this->is_signed) {
        const int16_t* data = (const int16_t*) d;
      } else {
      }
      break;
    default: abort(); /* unimplemented. */ break;
  }
}

static void
finalize(void* self)
{
  struct mcubes* mc = (struct mcubes*) self;
  if(fclose(mc->triangles) != 0) {
    perror("error closing triangle data file");
  }
  mc->triangles = NULL;
}

struct processor*
mcubes()
{
  struct mcubes* mc = calloc(sizeof(struct mcubes), 1);
  mc->init = init;
  mc->run = march;
  mc->fini = finalize;
  return (struct processor*)mc;
}

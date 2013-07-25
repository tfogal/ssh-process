#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
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
  FILE* cloud;
  size_t dims[3];
  float isoval;
  size_t slice; /* internal state: which slice are we processing? */
};

static void
init(void* self, const bool is_signed, const size_t bpc, const size_t d[3])
{
  struct mcubes* mc = (struct mcubes*) self;
  mc->is_signed = is_signed;
  mc->bpc = bpc;
  mc->slice = 0;
  mc->cloud = fopen(".point-cloud", "wb");
  memcpy(mc->dims, d, sizeof(size_t)*3);

  FILE* fp = fopen(".isovalue", "r");
  if(!fp) {
    fprintf(stderr, "could not read isovalue from '.isovalue'!\n");
    abort();
  }
  if(fscanf(fp, "%f", &mc->isoval) != 1) {
    fprintf(stderr, "could not scan FP from .isovalue!\n");
    abort();
  }
  fclose(fp);
}

/* samples the volume at the given position.  assumes 'this->dims' is a
 * size_t[3] (really, just size_t[2] will be okay) and that the volume is named
 * `data'. */
#define sample(x_,y_,z_) \
  data[z_*this->dims[1]*this->dims[0] + y_*this->dims[0] + x_]

/* @return true if the two given points straddle the given FP value. */
#define straddle(v_, x0,y0,z0, x1,y1,z1) \
  ({ typeof (*data) s0__ = sample(x0,y0,z0); \
     typeof (*data) s1__ = sample(x1,y1,z1); \
     (s0__ < v_ && v_ < s1__) || (s1__ < v_ && v_ < s0__); })

#define lerp(v, in0,in1, out0,out1) \
  (float)out0 + (v-in0) * ((float)out1-out0) / ((float)in1-in0)

/* need to add slice number to all the 'z's here! */
#define marching(dims_, isovalue)                                            \
  /* since we need four corners, indent by one.                              \
   * then we can be sure -1 is always a valid index. */                      \
  assert(dims_[0] > 2); assert(dims_[1] > 2);                                \
  for(size_t y=1; y < dims_[1]; ++y) {                                       \
    for(size_t x=1; x < dims_[0]; ++x) {                                     \
      /* we have 12 edges.  look at each one and decide if it's above or     \
       * below the isovalue. */                                              \
      /* first 8 are simple: the front face and back face. */                \
      for(size_t z=0; z < 2; ++z) {                                          \
        if(straddle(isovalue, x-1,y-1,z, x,y-1,z)) {                         \
          float point[3] = {                                                 \
            lerp(isovalue, sample(x-1,y-1,z),sample(x,y-1,z), x-1,x),        \
            y-1,                                                             \
            z + this->slice                                                  \
          };                                                                 \
          fwrite(point, sizeof(float), 3, this->cloud);                      \
        }                                                                    \
        if(straddle(isovalue, x-1,y,z, x,y,z)) {                             \
          float point[3] = {                                                 \
            lerp(isovalue, sample(x-1,y-1,z),sample(x,y-1,z), x-1,x),        \
            y,                                                               \
            z + this->slice                                                  \
          };                                                                 \
          fwrite(point, sizeof(float), 3, this->cloud);                      \
        }                                                                    \
        if(straddle(isovalue, x-1,y-1,z, x-1,y,z)) {                         \
          float point[3] = {                                                 \
            x-1,                                                             \
            lerp(isovalue, sample(x-1,y-1,z),sample(x-1,y,z), y-1,y),        \
            z + this->slice                                                  \
          };                                                                 \
          fwrite(point, sizeof(float), 3, this->cloud);                      \
        }                                                                    \
        if(straddle(isovalue, x,y-1,z, x,y,z)) {                             \
          float point[3] = {                                                 \
            x,                                                               \
            lerp(isovalue, sample(x,y-1,z),sample(x,y,z), y-1,y),            \
            z + this->slice                                                  \
          };                                                                 \
          fwrite(point, sizeof(float), 3, this->cloud);                      \
        }                                                                    \
      }                                                                      \
      /* next 4 are the edges that connect the front and back faces. */      \
      /* ... */                                                              \
      if(straddle(isovalue, x-1,y-1,0, x-1,y-1,1)) {                         \
        float point[3] = {                                                   \
          x-1,                                                               \
          y-1,                                                               \
          lerp(isovalue, sample(x-1,y-1,0),sample(x-1,y-1,1), this->slice,   \
                                                              this->slice+1) \
        };                                                                   \
        fwrite(point, sizeof(float), 3, this->cloud);                        \
      }                                                                      \
      if(straddle(isovalue, x-1,y,0, x-1,y,1)) {                             \
        float point[3] = {                                                   \
          x-1,                                                               \
          y,                                                                 \
          lerp(isovalue, sample(x-1,y,0),sample(x-1,y,1), this->slice,       \
                                                          this->slice+1)     \
        };                                                                   \
        fwrite(point, sizeof(float), 3, this->cloud);                        \
      }                                                                      \
      if(straddle(isovalue, x,y-1,0, x,y-1,1)) {                             \
        float point[3] = {                                                   \
          x,                                                                 \
          y-1,                                                               \
          lerp(isovalue, sample(x,y-1,0),sample(x,y-1,1), this->slice,       \
                                                          this->slice+1)     \
        };                                                                   \
        fwrite(point, sizeof(float), 3, this->cloud);                        \
      }                                                                      \
      if(straddle(isovalue, x,y,0, x,y,1)) {                                 \
        float point[3] = {                                                   \
          x,                                                                 \
          y,                                                                 \
          lerp(isovalue, sample(x,y,0),sample(x,y,1), this->slice,           \
                                                      this->slice+1)         \
        };                                                                   \
        fwrite(point, sizeof(float), 3, this->cloud);                        \
      }                                                                      \
    }                                                                        \
  }

static void
march(void* self, const void* d, const size_t nelems)
{
  const struct mcubes* this = (const struct mcubes*) self;

  if(nelems != this->dims[0]*this->dims[1]) {
    fprintf(stderr, "not enough data!\n");
    abort();
  }

  switch(this->bpc) {
    case 1:
      if(this->is_signed) {
        const int8_t* data = (const int8_t*) d;
        marching(this->dims, this->isoval);
      } else {
        const uint8_t* data = (const uint8_t*) d;
        marching(this->dims, this->isoval);
      }
      break;
    case 2:
      if(this->is_signed) {
        const int16_t* data = (const int16_t*) d;
        marching(this->dims, this->isoval);
      } else {
        const uint16_t* data = (const uint16_t*) d;
        marching(this->dims, this->isoval);
      }
      break;
    default: abort(); /* unimplemented. */ break;
  }
  {
    struct mcubes* mutable_this = (struct mcubes*) self;
    mutable_this->slice++;
  }
}

static void
finalize(void* self)
{
  struct mcubes* mc = (struct mcubes*) self;
  if(fclose(mc->cloud) != 0) {
    perror("error closing triangle data file");
  }
  mc->cloud = NULL;
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

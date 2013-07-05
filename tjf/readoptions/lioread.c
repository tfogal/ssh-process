#define _GNU_SOURCE
#include <aio.h>
#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "lioread.h"
#include "need.h"

static void
aio_info(const struct aiocb* aio)
{
  printf("[%p] filedes: %d\n[%p] offset: %zu\n[%p] buf: %p\n[%p] bytes: %zu\n"
         "[%p] opcode: %d\n[%p] event: %p\n",
         aio, aio->aio_fildes,
         aio, aio->aio_offset,
         aio, aio->aio_buf,
         aio, aio->aio_nbytes,
         aio, (int)aio->aio_lio_opcode,
         aio, &aio->aio_sigevent);
}

static bool
all_successful(struct aiocb* ioa[], size_t n)
{
  for(size_t i=0; i < n; ++i) {
    ssize_t susp = aio_suspend((const struct aiocb*const*)&ioa[i], 1, NULL);
    if(susp != 0) {
      perror("suspending");
      exit(EXIT_FAILURE);
    }
    ssize_t rv = aio_return(ioa[i]);
    if(rv != (int)ioa[i]->aio_nbytes) {
      aio_info(ioa[i]);
      fprintf(stderr, "err [%zu]: %d\n", i, (int)rv);
      return false;
    }
  }
  return true;
}

/** @returns true if a is a multiple of b */
__attribute__((pure)) static bool multiple(const size_t a, const size_t b)
{
  for(size_t i=1; i < 16; ++i) {
    if(a == b*i) { return true; }
  }
  return false;
}

static void
tune_aio()
{
  struct aioinit init;
  init.aio_threads = 4096;
  /*init.aio_num = 524288;*/
  init.aio_idle_time = 60;
  aio_init(&init);
}

void*
read_lio(const char* filename, size_t padding,
         const size_t dims[3], size_t bpc)
{
  assert(dims[0] > 0 && dims[1] > 0 && dims[2] > 0);

  tune_aio();

  int fd = needopen(filename, O_RDONLY | O_NOATIME);
  void* buffer = calloc(dims[0]*dims[1]*dims[2], bpc);
  if(buffer == NULL) {
    perror("allocation");
    exit(EXIT_FAILURE);
  }

  /* are we padding?  then the pad bits should give us a multiple of 4096. */
  /* should really be that it is a multiple of 4096, but currently all our data
   * has < 4096 byte scanlines. */
  assert(padding ? multiple((dims[0]*bpc)+padding, 4096) : true);

  struct sigevent nothing;
  nothing.sigev_notify = SIGEV_NONE;

  const size_t cols = dims[0] + (padding/bpc);

  struct aiocb** iolist = malloc(sizeof(struct aiocb*) * dims[1]*dims[2]);
  for(size_t z=0; z < dims[2]; ++z) {
    const size_t zoff = z * cols * dims[1];
    for(size_t y=0; y < dims[1]; ++y) {
      const size_t yoff = y * cols;
      const size_t off = zoff + yoff + 0 /* xoffset */;
      /* printf("offset: %zu\n", off); */
      iolist[z*dims[1] + y] = calloc(1, sizeof(struct aiocb));
      iolist[z*dims[1] + y]->aio_fildes = fd;
      iolist[z*dims[1] + y]->aio_offset = bpc*off;
      iolist[z*dims[1] + y]->aio_buf = (uint16_t*)buffer +
                                       (y*dims[0] + z*dims[0]*dims[1]);
      iolist[z*dims[1] + y]->aio_nbytes = dims[0] * bpc;
      iolist[z*dims[1] + y]->aio_lio_opcode = LIO_READ;
      iolist[z*dims[1] + y]->aio_sigevent = nothing;
    }
  }
#if 1
  printf("Enqueuing...\n");
  int lio_success = lio_listio(LIO_WAIT, iolist, dims[2]*dims[1], NULL);
  if(lio_success != 0) {
    perror("error enqueuing io group");
    exit(EXIT_FAILURE);
  }
  printf("complete.  testing for finality...\n");
  assert(all_successful(iolist, dims[2]*dims[1]));
#endif
  free(iolist);

  needclose(fd);
  return buffer;
}

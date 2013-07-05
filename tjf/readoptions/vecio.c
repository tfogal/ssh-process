/* this implementation was abandoned as useless: readv is for IO which will be
 * scattered in memory and contiguous on disk, but we have the opposite. */
#include <sys/uio.h>
#include "need.h"
#include "vecio.h"

void
read_vec(const char* filename, size_t padding,
         const size_t dims[3], size_t bpc)
{
  assert(dims[0] > 0 && dims[1] > 0 && dims[2] > 0);
  assert(padding ? multiple((dims[0]*bpc)+padding, 4096) : true);

  int fd = needopen(filename, O_RDONLY | O_NOATIME);
  void* buffer = calloc(dims[0]*dims[1]*dims[2], bpc);
  if(buffer == NULL) { perror("alloc"); exit(EXIT_FAILURE); }

  void* junkbuf = calloc(padding, 1); assert(junkbuf);

  const size_t cols = dims[0] + (padding/bpc);
  struct iovec** vec = malloc(sizeof(struct iovec*) * dims[1]*dims[2]*2);

  for(size_t z=0; z < dims[2]; ++z) {
    const size_t zoff = z * cols * dims[1];
    for(size_t y=0; y < dims[1]; ++y) {
      vec[2*(z*dims[1] + y) + 0] = calloc(1, sizeof(struct iovec));
      vec[2*(z*dims[1] + y) + 0]->iov_base = (uint16_t*)buffer +
                                     (y*dims[0] + z*dims[0]*dims[1]);
      vec[2*(z*dims[1] + y) + 0]->iov_len = dims[0] * bpc;
      /* extra padding is nonsense, read it into junk buffer */
      vec[2*(z*dims[1] + y) + 1] = calloc(1, sizeof(struct iovec));
      vec[2*(z*dims[1] + y) + 1]->iov_base = junkbuf;
      vec[2*(z*dims[1] + y) + 1]->iov_len = padding;
    }
  }
  ssize_t rrv = readv(fd, vec, dims[1]*dims[2]*2);
}

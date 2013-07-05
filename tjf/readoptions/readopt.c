#define _POSIX_C_SOURCE 200809L
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <libgen.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../../datasets.h"
#include "lioread.h"
#include "mpiread.h"
#include "need.h"

static void usage(const char* name) {
  fprintf(stderr, "%s - read every element a raw dataset\n", name);
  fprintf(stderr, "usage: %s [opts]\n", name);
  fprintf(stderr, "\n"
    "\t-f filename            file to read\n"
    "\t-b                     use lio_listio-based read\n"
    "\t-h                     this help information\n"
    "\t-m                     MPI-based read.  make sure to use mpiexec\n"
    "\t-p                     don't consider pad bytes\n"
  );
  exit(2);
}

/* A basename that doesn't suck.  You must free the string it returns. */
__attribute__((malloc)) static char* tjf_basename(const char* path) {
  /* need a copy: can't modify the argument. */
  char* copy = strdup(path);

  /* want to free our copy, but basename's return value might point into it.
   * so we basename the string and then copy the result into our 'retval' */
  char* retval = calloc(strlen(copy) + 4, 1);
  char* v = basename(copy);
  strcpy(retval, v);
  free(copy);

  return retval;
}

ssize_t
io(ssize_t(*f) (int, void*, size_t), int fd, void* buf, size_t len)
{
  size_t position = 0;
  char* buffer = buf;

  while(len > position) {
    ssize_t bytes = (f)(fd, ((char*)buffer) + position, len - position);
    switch(bytes) {
      case -1:
        if(EINTR == errno) { continue; }
        if(EAGAIN == errno || EWOULDBLOCK == errno) {
          struct pollfd pfd;
          pfd.fd = fd;
          pfd.events = (f == read) ? POLLIN : POLLOUT;
          (void)poll(&pfd, 1, -1); /* wait for IO ready signal */
          continue;
        }
        return 0;
      case 0: errno = EPIPE; return position;
      default:
        position += (size_t)bytes;
        break;
    }
  }
  assert(position == len);
  return position;
}

static void
read_by_scanline(const char* filename, size_t scanline, size_t padding,
                 const size_t dims[3])
{
  int fd = needopen(filename, O_RDONLY | O_NOATIME);
  void* buffer = calloc(scanline, sizeof(uint16_t));

  const size_t n_scanlines = dims[1] * dims[2];

  for(size_t sline=0; sline < n_scanlines; ++sline) {
    size_t rbytes = io(read, fd, buffer, scanline);
    assert(rbytes == scanline);
    /* seek over the nulls or emptiness. */
    off_t newpos = lseek(fd, padding, SEEK_CUR);
    if(newpos == -1) {
      perror("lseek skipping over null bytes");
      exit(EXIT_FAILURE);
    }
  }
  needclose(fd);
}

static void
ds_info(const struct dsinfo* ds)
{
#ifdef DEBUGGING
  printf("dset: %zu+%zu (%zu bytes), %zu bpc", ds->scanline_size,
         ds->add_bytes, ds->scanline_size+ds->add_bytes, ds->bpc);
  if(ds->signedness == SIGNED) { printf(" signed"); }
  else { printf(" unsigned"); }
  printf(" {%zu x %zu x %zu}\n", ds->dims[0], ds->dims[1], ds->dims[2]);
#else
  (void)ds;
#endif
}

static char* filename = NULL;
static bool use_lio = false;
static bool no_padding = false;
static bool compare = false;
static bool mpi = false;

int main(int argc, char* argv[]) {
  if(argc < 2) {
    usage(argv[0]);
  }

  int ch;
  while((ch = getopt(argc, argv, "f:bpcmh")) != -1) {
    switch(ch) {
      case 'f':
        filename = strdup(optarg);
        break;
      case 'h': usage(argv[0]); break;
      case 'b':
        use_lio = true;
        break;
      case 'm':
        mpi = true;
        break;
      case 'p':
        no_padding = true;
        break;
      default:
        usage(argv[0]);
    }
  }
  struct dsinfo* ds = NULL;
  {
    char* tmpname = tjf_basename(filename);
    ds = tjf_findds(tmpname);
    free(tmpname);
  }

  if(ds == NULL) {
    fprintf(stderr, "Don't know dataset '%s', bailing.\n", argv[1]);
    return EXIT_FAILURE;
  }
  ds_info(ds);

  if(no_padding) {
    ds->add_bytes = 0;
  }

  if(compare) {
    assert(use_lio == false); /* doesn't make sense, then */
  } else if(use_lio) {
    read_lio(filename, ds->add_bytes, ds->dims, ds->bpc);
  } else if(mpi) {
    const size_t expanded[3] = {
      ds->dims[0] + (ds->add_bytes / ds->bpc),
      ds->dims[1],
      ds->dims[2]
    };
    read_mpi(filename, ds->dims, expanded, ds->bpc);
  } else {
    read_by_scanline(filename, ds->scanline_size, ds->add_bytes, ds->dims);
  }
  free(filename);

  return EXIT_SUCCESS;
}

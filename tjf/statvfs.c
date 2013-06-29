#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/statvfs.h>

static struct statvfs svfs(const char* path) {
  static struct statvfs buf;
  int serr;

  do {
    serr = statvfs(path, &buf);
    if(serr == -1 && errno != EINTR) {
      perror("could not stat vfs");
      exit(EXIT_FAILURE);
    }
  } while(errno == EINTR);

  return buf;
}

int main(int argc, char* argv[]) {
  const char* fname = argv[0];
  if(argc == 2) {
    fname = argv[1];
  }
  struct statvfs sv = svfs(fname);

  printf("block size: %lu\n", sv.f_bsize);
  printf("fragment size: %lu\n", sv.f_frsize);
  printf("block count: %lu\n", (uint64_t)sv.f_blocks);
  printf("free blocks: %lu\n", (uint64_t)sv.f_bfree);
  printf("free blocks (unprivileged): %lu\n", (uint64_t)sv.f_bavail);
  printf("inodes: %lu\n", (uint64_t)sv.f_files);
  printf("free inodes: %lu\n", (uint64_t)sv.f_ffree);
  printf("free inodes (unprivileged): %lu\n", (uint64_t)sv.f_favail);
  printf("FS ID: %lu\n", sv.f_fsid);
  printf("mount flags: %lu\n", sv.f_flag);
  printf("maximum name length: %lu\n", sv.f_namemax);

  return EXIT_SUCCESS;
}

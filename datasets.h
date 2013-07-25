/* Contains information on datasets used in my study. */

#ifndef TJF_DATASETS_H
#define TJF_DATASETS_H

#include <stddef.h>

struct dsinfo {
  const char* filename;
  size_t scanline_size;
  size_t add_bytes;
  size_t bpc; /* bytes per component, not bits */
  enum { SIGNED, UNSIGNED } signedness;
  size_t dims[3];
};

extern struct dsinfo* tjf_findds(const char* filename);


#endif /* TJF_DATASETS_H */

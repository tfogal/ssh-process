#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datasets.h"

static struct dsinfo tjfds[] = {
  {"magnitude.nhdr.raw", 4050, 46, 2, UNSIGNED, {2025, 1600, 400}},
  /* nonsense, only for testing: */
  {"DynamicBrickingDS.cpp", 470, 42, 1, UNSIGNED, {470,1,1}},
  {"smalldata", 248, 8, 2, UNSIGNED, {81,10,5}},
};
static size_t num_ds = sizeof(tjfds) / sizeof(tjfds[0]);

/* find the dataset for the given filename (if it exists) */
struct dsinfo*
tjf_findds(const char* filename) {
  size_t i;
  for(i=0; i < num_ds; ++i) {
    const size_t len = strlen(tjfds[i].filename) + 64;
    char* pre = calloc(len, 1);
    snprintf(pre, len, "./%s", tjfds[i].filename);
    if(strcmp(tjfds[i].filename, filename) == 0 ||
       strcmp(pre, filename) == 0) {
      free(pre); pre = NULL;
      return &tjfds[i];
    }
    free(pre); pre = NULL;
  }
  return NULL;
}

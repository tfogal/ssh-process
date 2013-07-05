#ifndef TJF_LIOREAD_H
#define TJF_LIOREAD_H

extern void* read_lio(const char* filename, size_t padding,
                      const size_t dims[3], size_t bpc);

#endif /* TJF_LIOREAD_H */

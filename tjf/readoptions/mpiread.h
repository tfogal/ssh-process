#ifndef TJF_MPIREAD_H
#define TJF_MPIREAD_H

#include <unistd.h>

/* faux_dims >= real_dims */
extern void read_mpi(const char* filename,
                     const size_t real_dims[3],
                     const size_t faux_dims[3],
                     const size_t bpc);

#endif /* TJF_MPIREAD_H */

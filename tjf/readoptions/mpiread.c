#include <assert.h>
#include <mpi.h>
#include "mpiread.h"

static void
init_mpi()
{
  int argc = 0;
  char* argv[] = { NULL };
  MPI_Init(&argc, (char***)&argv);
}

static void
kill_mpi()
{
  MPI_Finalize();
}

void
read_mpi(const char* filename,
         const size_t real_dims[3],
         const size_t faux_dims[3],
         const size_t bpc)
{
  assert(real_dims[0] <= faux_dims[0]);
  assert(real_dims[1] <= faux_dims[1]);
  assert(real_dims[2] <= faux_dims[2]);

  init_mpi();
  kill_mpi();
}

#include <assert.h>
#include <inttypes.h>
#include <mpi.h>
#include <stdio.h>
#include "mpiread.h"

/* give an MPI call non-lame semantics, so one doesn't need to create a stack
 * variable. */
__attribute__((pure)) static int
mpi_dumb_int(int(*f)(MPI_Comm, int*))
{
  int x;
  f(MPI_COMM_WORLD, &x);
  return x;
}
__attribute__((pure)) static uint64_t
rank()
{
  return mpi_dumb_int(MPI_Comm_rank);
}
__attribute__((pure)) static uint64_t
size()
{
  return mpi_dumb_int(MPI_Comm_size);
}

static void
init_mpi()
{
  int argc = 0;
  char* argv[] = { NULL };
  MPI_Init(&argc, (char***)&argv);
}

static MPI_Datatype
create_type(const size_t real_dims[3], const size_t faux_dims[3])
{
  assert(real_dims[0] <= faux_dims[0] && real_dims[1] <= faux_dims[1]);
  /* MPI needs data in an array of ints.  gah. */
  /* const */ int sizes[3] = {faux_dims[0], faux_dims[1], faux_dims[2]};
  /* we split the array in Z based on the number of procs we've got. */
  int subsize[3] = {
    real_dims[0],
    real_dims[1],
    real_dims[2] / size()
  };
  /* the array offsets that this process' array starts at. */
  /* const */ int offsets[3] = {
    0, 0, subsize[2] * rank()
  };
  printf("My array is {%d--%d, %d--%d, %d--%d} within {%d, %d, %d}\n",
         offsets[0], offsets[0]+subsize[0],
         offsets[1], offsets[1]+subsize[1],
         offsets[2], offsets[2]+subsize[2],
         sizes[0], sizes[1], sizes[2]);
  MPI_Datatype filetype;
  int typeerr = MPI_Type_create_subarray(3, sizes, subsize, offsets,
                                         MPI_ORDER_C, MPI_UNSIGNED_SHORT,
                                         &filetype);
  if(typeerr != 0) {
    fprintf(stderr, "mpi error: %d\n", typeerr);
    MPI_Abort(MPI_COMM_WORLD, typeerr);
  }
  return filetype;
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
  MPI_Datatype subarray = create_type(real_dims, faux_dims);

  MPI_File fh;
  int oerr = MPI_File_open(MPI_COMM_WORLD, (char*)filename, MPI_MODE_RDONLY,
                           MPI_INFO_NULL, &fh);
  if(oerr != 0) {
    fprintf(stderr, "[%lu] could not open %s\n", rank(), filename);
    MPI_Abort(MPI_COMM_WORLD, oerr);
  }

  int ferr = MPI_Type_free(&subarray);
  if(ferr != 0) {
    fprintf(stderr, "error cleaning data type: %d\n", ferr);
    MPI_Abort(MPI_COMM_WORLD, ferr);
  }

  int clerr = MPI_File_close(&fh);
  if(clerr != 0) {
    fprintf(stderr, "error closing file.\n");
    MPI_Abort(MPI_COMM_WORLD, clerr);
  }
  kill_mpi();
}

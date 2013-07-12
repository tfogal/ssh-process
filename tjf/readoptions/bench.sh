#!/bin/sh

repetitions=10
prf="perf stat --repeat ${repetitions} --sync --pre ./write3.sh --"
function runtest {
  sync
  sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
  ${prf} $@ 2> .timing-results
  grep elapsed .timing-results
}
function runtread {
  fname="$1"
  runtest "./testread -f ${fname} -p"
}

holy="../../magnitude.nhdr.raw"
original="${HOME}/data/aligned-testdata/magnitude.nhdr.raw"

echo "Running with ${repetitions} repetitions."

echo "original data, standard reads:"
runtest "./testread -f ${original} -p"

echo "hole-y data, standard reads:"
runtest "./testread -f ${holy}"

echo "original data, MPI with 4 procs:"
runtest "mpiexec -n 4 -- ./testread -m -f ${original} -p"

echo "hole-y data, MPI with 4 procs:"
runtest "mpiexec -n 4 -- ./testread -m -f ${holy}"

#prf="perf stat --repeat ${repetitions} --sync --pre ./write3.sh --"
#echo "original data, batch:"
#runtest "./testread -f ${original} -p"
#
#echo "hole-y data, batch:"
#runtest "./testread -f ${holy}"

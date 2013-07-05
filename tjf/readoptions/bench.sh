#!/bin/sh

repetitions=2
prf="perf stat --repeat ${repetitions} --sync --pre ./write3.sh --"
function runtest {
  sync
  sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
  ${prf} $@ | grep elapsed
}
function runtread {
  fname="$1"
  runtest "./testread -f ${fname} -p"
}

holy="../../magnitude.nhdr.raw"
original="${HOME}/data/aligned-testdata/magnitude.nhdr.raw"

echo "original data, stupid way:"
runtest "./testread -f ${original} -p"

echo "hole-y data, stupid way:"
runtest "./testread -f ${holy}"

#prf="perf stat --repeat ${repetitions} --sync --pre ./write3.sh --"
#echo "original data, batch:"
#runtest "./testread -f ${original} -p"
#
#echo "hole-y data, batch:"
#runtest "./testread -f ${holy}"

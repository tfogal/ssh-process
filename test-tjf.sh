#!/bin/sh

scp="./scp -S ./ssh"
remote="grid16.ivda.uni-saarland.de"
remote="shell.sci.utah.edu"
testfile="magnitude.nhdr.raw"

# -T: enable my stuff
# -a: enable giving advice
# -h: create 'holy' file.
# -m: run marching cubes

# simple test: just grabbing a small file
#${scp} -v -T ${remote}:DynamicBrickingDS.cpp .
#${scp} -v -T -a ${remote}:DynamicBrickingDS.cpp .
#${scp} -v -T -a -h ${remote}:DynamicBrickingDS.cpp .
if test -f ".isovalue-${testfile}" ; then
  rm -f .isovalue
  ln -s ".isovalue-${testfile}" .isovalue
fi
${scp} -v -T -m ${remote}:${testfile} .

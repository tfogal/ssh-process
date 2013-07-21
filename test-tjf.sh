#!/bin/sh

scp="./scp -S ./ssh"
remote="grid16.ivda.uni-saarland.de"

# -T: enable my stuff
# -a: enable giving advice
# -h: create 'holy' file.

# simple test: just grabbing a small file
#${scp} -v -T ${remote}:DynamicBrickingDS.cpp .
#${scp} -v -T -a ${remote}:DynamicBrickingDS.cpp .
#${scp} -v -T -a -h ${remote}:DynamicBrickingDS.cpp .
${scp} -v -T -m ${remote}:smalldata .

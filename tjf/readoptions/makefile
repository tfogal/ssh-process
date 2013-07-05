MPICC=mpicc
CFLAGS:=-std=c99 -Wall -Wextra -ggdb
OBJ=need.o readopt.o ../../datasets.o lioread.o mpiread.mo
LIBS=-lrt

all: $(OBJ) testread

testread: readopt.o need.o ../../datasets.o lioread.o mpiread.mo
	$(MPICC) $^ -o $@ $(LIBS)

clean:
	rm -f testread $(OBJ)

%.mo: %.c
	$(MPICC) -c $(CFLAGS) $^ -o $@

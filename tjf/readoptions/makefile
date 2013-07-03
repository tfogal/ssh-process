CFLAGS:=-std=c99 -Wall -Wextra
OBJ=readopt.o ../../datasets.o

all: $(OBJ) testread

testread: readopt.o ../../datasets.o
	$(CC) $^ -o $@ $(LIBS)

clean:
	rm -f testread $(OBJ)

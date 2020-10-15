CC=g++
MPI_CC=mpic++
CFLAGS=-std=c++17

sequential-build:
	$(CC) sort-seq.cpp -o sort-seq.o $(CFLAGS)

sequential: sequential-build
	./sort-seq.o $(number-vectors) $(vector-size)

mpi-build:
	$(MPI_CC) sort-mpi.cpp -o sort-mpi.o $(CFLAGS)

mpi: mpi-build
	mpirun -np $(np) --hostfile ../hosts ./sort-mpi.o $(number-vectors) $(vector-size)

.PHONY : clean
clean:
	rm -rf *.o

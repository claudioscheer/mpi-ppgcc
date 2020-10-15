CC=g++
MPI_CC=mpic++
CFLAGS=-std=c++17

sequential-build:
	$(CC) lr-seq.cpp -o lr-seq.o $(CFLAGS)

sequential: sequential-build
	./lr-seq.o $(number-points)

mpi-build:
	$(MPI_CC) lr-mpi.cpp -o lr-mpi.o $(CFLAGS)

mpi: mpi-build
	mpirun -np $(np) --hostfile ../hosts ./lr-mpi.o $(number-points) $(granularity)

.PHONY : clean
clean:
	rm -rf *.o

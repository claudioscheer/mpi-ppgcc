CC=g++
MPI_CC=mpic++
CFLAGS=-std=c++17

sequential-build:
	$(CC) lr-seq.cpp -o lr-seq.o $(CFLAGS)

sequential: sequential-build
	./lr-seq.o $(bucket-size) $(number-buckets)

mpi-build:
	$(MPI_CC) lr-mpi.cpp -o lr-mpi.o $(CFLAGS)

mpi: mpi-build
	mpirun -np 16 --oversubscribe ./lr-mpi.o $(bucket-size)

.PHONY : clean
clean:
	rm -rf *.o

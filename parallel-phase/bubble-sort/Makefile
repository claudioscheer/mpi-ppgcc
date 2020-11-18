CC=gcc
MPI_CC=mpicc
CFLAGS=

sequential-build:
	$(CC) sort-seq.c -o sort-seq.o $(CFLAGS)

sequential: sequential-build
	./sort-seq.o $(vector-size)

mpi-build:
	$(MPI_CC) sort-mpi.c -o sort-mpi.o $(CFLAGS)

mpi: mpi-build
	mpirun -np $(np) --oversubscribe ./sort-mpi.o $(vector-size) $(percentage-items-exchange)

.PHONY : clean
clean:
	rm -rf *.o

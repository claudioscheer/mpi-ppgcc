CC=g++
MPI_CC=mpic++
CFLAGS=-std=c++17

dataset-generator-build:
	$(CC) dataset-generator.cpp -o dataset-generator.o

dataset-generator-small: dataset-generator-build
	./dataset-generator.o 90 180 900 5

dataset-generator-large: dataset-generator-build
	./dataset-generator.o 9000000 18000000 90000000 50

sequential-build:
	$(CC) lr-seq.cpp -o lr-seq.o $(CFLAGS)

sequential: sequential-build
	./lr-seq.o $(n)

mpi-build:
	$(MPI_CC) lr-mpi.cpp -o lr-mpi.o $(CFLAGS)

mpi: mpi-build
	mpirun -np 16 --oversubscribe ./lr-mpi.o $(n)

.PHONY : clean
clean:
	rm -rf *.o

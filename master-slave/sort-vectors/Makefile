default: sort_vector

clean:
	@rm -f main.o

sort_vector:
	mpic++ main.cpp -o main.o
	mpirun -np 16 --oversubscribe ./main.o

.PHONY: all clean

CC=g++
CFLAGS=-std=c++17

dataset-generator-build:
	$(CC) dataset-generator.cpp -o dataset-generator.o

dataset-generator-small: dataset-generator-build
	./dataset-generator.o 90 180 900 5

dataset-generator-large: dataset-generator-build
	./dataset-generator.o 9000000 18000000 90000000 50

build:
	$(CC) lr-seq.cpp -o lr-seq.o $(CFLAGS)

sequential: build
	./lr-seq.o $(n)

.PHONY : clean
clean:
	rm -rf *.o

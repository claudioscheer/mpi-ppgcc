#include <stdio.h>

int *get_vector(int vector_size, int *v) {
    for (int i = 0; i < vector_size; i++) {
        v[i] = vector_size - i;
    }
    return v;
}

int *get_vector_offset(int vector_size, int *v, int offset) {
    int init = vector_size * offset;
    int end = vector_size * offset + vector_size;
    for (int i = init; i < end; i++) {
        v[i] = vector_size - i;
    }
    return v;
}

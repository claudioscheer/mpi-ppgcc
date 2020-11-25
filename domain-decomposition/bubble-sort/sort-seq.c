#include "dataset-generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

float time_difference_msec(struct timeval t0, struct timeval t1) {
    return (t1.tv_sec - t0.tv_sec) * 1000.0f +
           (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

int is_vector_sorted(int *vector, int vector_size) {
    int last_element = -1;
    for (int i = 0; i < vector_size; i++) {
        if (last_element > vector[i]) {
            return 0;
        }
        last_element = vector[i];
    }
    return 1;
}

int *bubble_sort(int vector_size, int *vector_unsorted) {
    int swapped = 1;

    int i = 0;
    while ((i < (vector_size - 1)) & swapped) {
        swapped = 0;
        for (int j = 0; j < vector_size - i - 1; j++)
            if (vector_unsorted[j] > vector_unsorted[j + 1]) {
                int temp = vector_unsorted[j];
                vector_unsorted[j] = vector_unsorted[j + 1];
                vector_unsorted[j + 1] = temp;
                swapped = 1;
            }
        i++;
    }

    return vector_unsorted;
}

int main(int argc, char **argv) {
    int vector_size = atoi(argv[1]);

    int vector_unsorted[vector_size];
    get_vector(vector_size, vector_unsorted);

    struct timeval t0;
    struct timeval t1;

    gettimeofday(&t0, 0);
    int *vector_sorted = bubble_sort(vector_size, vector_unsorted);
    gettimeofday(&t1, 0);

    float total_time = time_difference_msec(t0, t1);

    printf("Vector sorted: %d\n", is_vector_sorted(vector_sorted, vector_size));
    printf("Vector size: %d\n", vector_size);
    printf("Time sort (ms): %f\n", total_time);

    return 0;
}

// TO DO
// - If a process has failed, stop broadcasting.
// - Try different sizes of items being transmitted.
// - Count iterations number.
// - Apply bubble sort only once. Use interleave.

#include "dataset-generator.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

float time_difference_msec(struct timeval t0, struct timeval t1) {
    return (t1.tv_sec - t0.tv_sec) * 1000.0f +
           (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

// a must be the smallest vector. Otherwise, the interleave will be wrong.
int *interleave_vector(int *vector, int a_init_index, int a_end_index,
                       int b_init_index, int b_end_index) {
    int *result =
        (int *)malloc(sizeof(int) * ((a_end_index - a_init_index) +
                                     (b_end_index - b_init_index) + 2));

    int i = 0, a = a_init_index, b = b_init_index;
    while (a <= a_end_index || b <= b_end_index) {
        if (a <= a_end_index && (vector[a] < vector[b] || b > b_end_index)) {
            result[i] = vector[a];
            a++;
        } else {
            result[i] = vector[b];
            b++;
        }
        i++;
    }

    return result;
}

void bubble_sort(int vector_size, int *vector_unsorted) {
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
}

int main(int argc, char **argv) {
    int vector_size = atoi(argv[1]);
    float percentage_items_exchange = atof(argv[2]);

    MPI_Status status;
    int my_rank;
    int num_processes;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    if (vector_size % num_processes) {
        printf("Error: vector size must be a multiple of the number of "
               "processes.\n");
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    double t0;
    double t1;
    t0 = MPI_Wtime();

    int subvector_size = vector_size / num_processes;
    int number_items_shared = subvector_size * percentage_items_exchange;
    int subvector[subvector_size + number_items_shared];
    get_vector_offset(vector_size, subvector_size, subvector, my_rank);

    int process_status[num_processes];

    int done = 0;
    while (!done) {
        // --------------------FIRST PHASE--------------------
        bubble_sort(subvector_size, subvector);
        // --------------------FIRST PHASE--------------------

        // --------------------SECOND PHASE--------------------
        if (my_rank < num_processes - 1) {
            MPI_Send(&subvector[subvector_size - 1], 1, MPI_INT, my_rank + 1, 0,
                     MPI_COMM_WORLD);
        }
        if (my_rank > 0) {
            int largest_number_left;
            MPI_Recv(&largest_number_left, 1, MPI_INT, my_rank - 1, 0,
                     MPI_COMM_WORLD, &status);
            process_status[my_rank] = subvector[0] > largest_number_left;
        } else {
            process_status[my_rank] = 1;
        }

        for (int i = 0; i < num_processes; i++) {
            MPI_Bcast(&process_status[i], 1, MPI_INT, i, MPI_COMM_WORLD);
            // Stop when receive zero.
        }

        int vector_sorted = 1;
        for (int i = 0; i < num_processes; i++) {
            vector_sorted &= process_status[i];
            // Stop when receive zero.
        }
        if (vector_sorted) {
            done = 1;
            break;
        }
        // --------------------SECOND PHASE--------------------

        // --------------------THIRD PHASE--------------------
        if (my_rank > 0) {
            MPI_Send(&subvector[0], number_items_shared, MPI_INT, my_rank - 1,
                     0, MPI_COMM_WORLD);
        }
        if (my_rank < num_processes - 1) {
            MPI_Recv(&subvector[subvector_size], number_items_shared, MPI_INT,
                     my_rank + 1, 0, MPI_COMM_WORLD, &status);

            int *interleaved_vector = interleave_vector(
                subvector, subvector_size - number_items_shared,
                subvector_size - 1, subvector_size,
                subvector_size + number_items_shared - 1);

            MPI_Send(&interleaved_vector[number_items_shared],
                     number_items_shared, MPI_INT, my_rank + 1, 0,
                     MPI_COMM_WORLD);

            for (int i = 0; i < number_items_shared; i++) {
                subvector[subvector_size - number_items_shared + i] =
                    interleaved_vector[i];
            }
        }
        if (my_rank > 0) {
            MPI_Recv(&subvector[0], number_items_shared, MPI_INT, my_rank - 1,
                     0, MPI_COMM_WORLD, &status);
        }
        // --------------------THIRD PHASE--------------------
        /* for (int i = 0; i < subvector_size + 1; i++) { */
        /*     printf("%d ", subvector[i]); */
        /* } */
        /* printf("----- %d", my_rank); */
        /* printf("\n"); */
        /* fflush(stdout); */
        /* break; */
    }

    t1 = MPI_Wtime();
    double total_time = t1 - t0;

    if (my_rank == 0) {
        printf("Vector size: %d\n", vector_size);
        printf("Time sort (ms): %f\n", total_time);
    }

    MPI_Finalize();

    return 0;
}

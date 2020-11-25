// TO DO
// - Count iterations number.

#include "dataset-generator.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define DEBUG 0
#define OPTIMIZE_BROADCAST 1
#define BUBBLE_SORT_ONLY_ONCE 1

void interleave_vector(int *vector, int a_init_index, int a_end_index,
                       int b_init_index, int b_end_index, int *result) {
    if (a_end_index - a_init_index > b_end_index - b_init_index) {
        int temp = a_init_index;
        a_init_index = b_init_index;
        b_init_index = temp;
        temp = a_end_index;
        a_end_index = b_end_index;
        b_end_index = temp;
    }

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

    // Avoid allocating a new vector in each iteration.
    int process_status[num_processes];
    int interleaved_shared_vector_pre_defined[number_items_shared * 2];
#if BUBBLE_SORT_ONLY_ONCE == 1
    int interleaved_subvector_pre_defined[subvector_size];
    int bubble_sort_executed = 0;
#endif

#if DEBUG == 1
    int loop_index = 0;
#endif

    int done = 0;
    while (!done) {
        // --------------------FIRST PHASE--------------------
#if BUBBLE_SORT_ONLY_ONCE == 1
        if (!bubble_sort_executed) {
            bubble_sort(subvector_size, subvector);
            bubble_sort_executed = 1;
        } else {
            interleave_vector(subvector, number_items_shared,
                              subvector_size - number_items_shared - 1,
                              subvector_size - number_items_shared,
                              subvector_size - 1,
                              interleaved_subvector_pre_defined);
            for (int i = 0; i < subvector_size - number_items_shared; i++) {
                subvector[i + number_items_shared] =
                    interleaved_subvector_pre_defined[i];
            }

            interleave_vector(subvector, 0, number_items_shared - 1,
                              number_items_shared, subvector_size - 1,
                              interleaved_subvector_pre_defined);
            for (int i = 0; i < subvector_size; i++) {
                subvector[i] = interleaved_subvector_pre_defined[i];
            }
        }
#else
        bubble_sort(subvector_size, subvector);
#endif
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
#if OPTIMIZE_BROADCAST == 1
            if (!process_status[i]) {
                break;
            }
#endif
        }

        int vector_sorted = 1;
        for (int i = 0; i < num_processes; i++) {
            vector_sorted &= process_status[i];
            if (!vector_sorted) {
                break;
            }
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

            interleave_vector(subvector, subvector_size - number_items_shared,
                              subvector_size - 1, subvector_size,
                              subvector_size + number_items_shared - 1,
                              interleaved_shared_vector_pre_defined);

            MPI_Send(
                &interleaved_shared_vector_pre_defined[number_items_shared],
                number_items_shared, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD);

            for (int i = 0; i < number_items_shared; i++) {
                subvector[subvector_size - number_items_shared + i] =
                    interleaved_shared_vector_pre_defined[i];
            }
        }
        if (my_rank > 0) {
            MPI_Recv(&subvector[0], number_items_shared, MPI_INT, my_rank - 1,
                     0, MPI_COMM_WORLD, &status);
        }
        // --------------------THIRD PHASE--------------------
#if DEBUG == 1
        if (loop_index == 0) {
            for (int i = 0; i < subvector_size; i++) {
                printf("%d ", subvector[i]);
            }
            printf("----- %d", my_rank);
            printf("\n");
            fflush(stdout);
        }
        loop_index++;
#endif
    }

    t1 = MPI_Wtime();
    double total_time = (t1 - t0) * 1000;

#if DEBUG == 1
    if (my_rank > 0) {
        MPI_Send(&subvector[0], subvector_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    } else {
        int final_vector[vector_size];
        for (int i = 0; i < subvector_size; i++) {
            final_vector[i] = subvector[i];
        }

        for (int i = 1; i < num_processes; i++) {
            int subvector_received[subvector_size];
            MPI_Recv(&subvector_received[0], subvector_size, MPI_INT, i, 0,
                     MPI_COMM_WORLD, &status);

            for (int j = 0; j < subvector_size; j++) {
                final_vector[subvector_size * i + j] = subvector_received[j];
            }
        }

        printf("Final vector:\n");
        for (int i = 0; i < vector_size; i++) {
            printf("%d ", final_vector[i]);
        }
        printf("\n");

        printf("DEBUG: %d\n", DEBUG);
        printf("OPTIMIZE_BROADCAST: %d\n", OPTIMIZE_BROADCAST);
        printf("BUBBLE_SORT_ONLY_ONCE: %d\n", BUBBLE_SORT_ONLY_ONCE);
        printf("Vector size: %d\n", vector_size);
        printf("Time sort (ms): %f\n", total_time);
    }
#else
    if (my_rank == 0) {
        printf("DEBUG: %d\n", DEBUG);
        printf("OPTIMIZE_BROADCAST: %d\n", OPTIMIZE_BROADCAST);
        printf("BUBBLE_SORT_ONLY_ONCE: %d\n", BUBBLE_SORT_ONLY_ONCE);
        printf("Vector size: %d\n", vector_size);
        printf("Time sort (ms): %f\n", total_time);
    }
#endif

    MPI_Finalize();

    return 0;
}

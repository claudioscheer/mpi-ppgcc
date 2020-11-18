#include "dataset-generator.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

float time_difference_msec(struct timeval t0, struct timeval t1) {
    return (t1.tv_sec - t0.tv_sec) * 1000.0f +
           (t1.tv_usec - t0.tv_usec) / 1000.0f;
}

int *interleave_vectors(int a_size, int *a_vector, int b_size, int *b_vector) {
    int *result = (int *)malloc(sizeof(int) * (a_size + b_size));

    int *small_vector, *large_vector;
    int min, max;
    if (a_size < b_size) {
        small_vector = a_vector;
        large_vector = b_vector;
        min = a_size;
        max = b_size;
    } else {
        small_vector = b_vector;
        large_vector = a_vector;
        min = b_size;
        max = a_size;
    }

    int s = 0, l = 0;
    while (s < min) {
        if (small_vector[s] < large_vector[l] || l >= max) {
            result[s + l] = small_vector[s];
            s++;
        } else {
            result[s + l] = large_vector[l];
            l++;
        }
    }

    while (l < max) {
        result[s + l] = large_vector[l];
        l++;
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
    int subvector[subvector_size];
    get_vector_offset(vector_size, subvector_size, subvector, my_rank);

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
        int my_status;
        if (my_rank > 0) {
            int largest_number_left;
            MPI_Recv(&largest_number_left, 1, MPI_INT, my_rank - 1, 0,
                     MPI_COMM_WORLD, &status);
            my_status = subvector[0] > largest_number_left;
        } else {
            my_status = 1;
        }
        MPI_Bcast(&my_status, 1, MPI_INT, my_rank, MPI_COMM_WORLD);

        int global_status = my_status;
        for (int i = 0; i < num_processes; i++) {
            if (i == my_rank) {
                continue;
            }
            int neighbor_status;
            MPI_Bcast(&neighbor_status, 1, MPI_INT, i, MPI_COMM_WORLD);
            global_status &= neighbor_status;
            if (!global_status) {
                break;
            }
        }

        if (global_status) {
            done = 1;
            break;
        }
        // --------------------SECOND PHASE--------------------

        // --------------------THIRD PHASE--------------------
        int number_items_shared = subvector_size * percentage_items_exchange;

        if (my_rank > 0) {
            MPI_Send(&subvector[0], number_items_shared, MPI_INT, my_rank - 1,
                     0, MPI_COMM_WORLD);
        }
        int shared_piece_subvector[number_items_shared];
        if (my_rank < num_processes - 1) {
            MPI_Recv(shared_piece_subvector, number_items_shared, MPI_INT,
                     my_rank + 1, 0, MPI_COMM_WORLD, &status);

            // Get the last part of the subvector.
            int current_piece_subvector[number_items_shared];
            for (int i = 0; i < number_items_shared; i++) {
                current_piece_subvector[number_items_shared - i - 1] =
                    subvector[subvector_size - i - 1];
            }

            int *interleaved_vector = interleave_vectors(
                number_items_shared, shared_piece_subvector,
                number_items_shared, current_piece_subvector);

            MPI_Send(&interleaved_vector[number_items_shared],
                     number_items_shared, MPI_INT, my_rank + 1, 0,
                     MPI_COMM_WORLD);

            for (int i = 0; i < number_items_shared; i++) {
                subvector[subvector_size - number_items_shared + i] =
                    interleaved_vector[i];
            }
        }
        if (my_rank > 0) {
            MPI_Recv(shared_piece_subvector, number_items_shared, MPI_INT,
                     my_rank - 1, 0, MPI_COMM_WORLD, &status);

            for (int i = 0; i < number_items_shared; i++) {
                subvector[i] = shared_piece_subvector[i];
            }
        }
        // --------------------THIRD PHASE--------------------
    }

    t1 = MPI_Wtime();
    double total_time = t1 - t0;

    // --------------------AGGREGATES RESULTS ALL PROCESSES--------------------
    if (my_rank > 0) {
        MPI_Send(&subvector[0], subvector_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&total_time, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        int final_vector[vector_size];
        for (int i = 0; i < subvector_size; i++) {
            final_vector[i] = subvector[i];
        }

        for (int i = 1; i < num_processes; i++) {
            int subvector_received[subvector_size];
            MPI_Recv(subvector_received, subvector_size, MPI_INT, i, 0,
                     MPI_COMM_WORLD, &status);
            double total_time_received;
            MPI_Recv(&total_time_received, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD,
                     &status);

            for (int j = 0; j < subvector_size; j++) {
                final_vector[subvector_size * i + j] = subvector_received[j];
            }
            total_time += total_time_received;
        }

        printf("Final vector:\n");
        for (int i = 0; i < vector_size; i++) {
            printf("%d ", final_vector[i]);
        }
        printf("\n");

        printf("Vector size: %d\n", vector_size);
        printf("Time sort (ms): %f\n", total_time);
    }
    // --------------------AGGREGATES RESULTS ALL PROCESSES--------------------

    MPI_Finalize();

    return 0;
}

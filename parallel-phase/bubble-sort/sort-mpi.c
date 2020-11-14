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
    int *result;
    result = (int *)malloc(sizeof(int) * (a_size + b_size));

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
    int i = 0, s = 0, l = 0;
    while (s < min) {
        if (small_vector[s] < large_vector[l]) {
            result[i] = small_vector[s++];
        } else {
            result[i] = large_vector[l++];
        }
        i = s + l;
    }

    while (l < max) {
        result[i++] = large_vector[l++];
    }

    return result;
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
    int subvector_unsorted[subvector_size];
    get_vector_offset(subvector_size, subvector_unsorted, my_rank);

    int done = 0;
    while (!done) {
        int *subvector_sorted = bubble_sort(subvector_size, subvector_unsorted);

        if (my_rank != num_processes - 1) {
            MPI_Send(&subvector_sorted[subvector_size - 1], 1, MPI_INT,
                     my_rank + 1, 0, MPI_COMM_WORLD);
        }
        int largest_number_left;
        if (my_rank != 0) {
            MPI_Recv(&largest_number_left, 1, MPI_INT, my_rank - 1, 0,
                     MPI_COMM_WORLD, &status);
        }

        // Send my status to all workers.
        int my_status = subvector_sorted[0] > largest_number_left;
        MPI_Bcast(&my_status, 1, MPI_INT, my_rank, MPI_COMM_WORLD);

        // Check the status of all workers.
        int global_status = 1;
        for (int i = 0; i < num_processes; i++) {
            if (i == my_rank) {
                continue;
            }
            int neighbor_status;
            MPI_Bcast(&neighbor_status, 1, MPI_INT, i, MPI_COMM_WORLD);
            global_status &= neighbor_status;
            // If there is a neighbor with a bad status, stop receiving
            // broadcasts.
            if (!global_status) {
                break;
            }
        }

        if (global_status) {
            done = 1;
            break;
        }

        int number_items_share_left =
            subvector_size * percentage_items_exchange;

        if (my_rank != 0) {
            MPI_Send(&subvector_sorted[0], number_items_share_left, MPI_INT,
                     my_rank - 1, 0, MPI_COMM_WORLD);
        }
        if (my_rank != num_processes - 1) {
            int shared_piece_subvector[number_items_share_left];
            MPI_Recv(&shared_piece_subvector[0], number_items_share_left,
                     MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, &status);

            /* join shared_piece_subvector and subvector_sorted */
        }
    }

    t1 = MPI_Wtime();
    double total_time = t1 - t0;

    printf("Vector size: %d\n", vector_size);
    printf("Time sort (ms): %f\n", total_time);

    MPI_Finalize();

    return 0;
}

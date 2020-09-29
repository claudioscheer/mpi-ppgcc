#include "mpi.h"
#include <iostream>

int main(int argc, char **argv) {
    int my_rank;       // Process id.
    int num_processes; // Number of processes.
    int tag = 1;

    MPI_Status status; // Return status.
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    if (my_rank != 0) {
        int master = 0;
        int value;
        MPI_Recv(&value, 1, MPI_INT, master, tag, MPI_COMM_WORLD, &status);
        value *= value;
        MPI_Send(&value, 1, MPI_INT, master, tag, MPI_COMM_WORLD);
    } else {
        double start_time = MPI_Wtime();
        for (int i = 1; i < num_processes; i++) {
            MPI_Send(&i, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
        }
        for (int i = 1; i < num_processes; i++) {
            int result;
            MPI_Recv(&result, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
            printf("%d - %d\n", i, result);
        }
        double end_time = MPI_Wtime();
        printf("Execution time: %f seconds\n", end_time - start_time);
    }
    MPI_Finalize();
    return 0;
}

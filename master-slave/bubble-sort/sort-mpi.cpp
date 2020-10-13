#include "dataset-generator.cpp"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <sstream>
#include <tuple>
#include <vector>

using namespace std;

vector<vector<int>> load_dataset(int number_vectors, int vector_size) {
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    vector<vector<int>> vectors =
        dataset::get_dataset(number_vectors, vector_size);
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    double total_time =
        chrono::duration_cast<chrono::duration<double>>(end - begin).count();
    cout << "Time load dataset (s): " << total_time << endl;
    return vectors;
}

vector<int> bubble_sort(vector<int> v) {
    int n = v.size();
    int c = 0;
    int temp;
    int swapped = 1;

    while ((c < (n - 1)) & swapped) {
        swapped = 0;
        for (int d = 0; d < n - c - 1; d++)
            if (v.at(d) > v.at(d + 1)) {
                temp = v.at(d);
                v.at(d) = v.at(d + 1);
                v.at(d + 1) = temp;
                swapped = 1;
            }
        c++;
    }

    return v;
}

int main(int argc, char **argv) {
    int number_vectors = atoi(argv[1]);
    int vector_size = atoi(argv[2]);

    int vector_tag = 1;
    int kill_tag = 2;
    int request_vector_tag = 3;

    MPI_Status status;
    int my_rank;
    int num_processes;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    if (my_rank != 0) {
        int master = 0;
        int ask_for_message = 1;
        int kill_flag = 0;
        while (!kill_flag) {
            if (ask_for_message) {
                // Will only send a new request when the last request was
                // already processed.
                MPI_Send(&ask_for_message, 1, MPI_INT, master,
                         request_vector_tag, MPI_COMM_WORLD);
                ask_for_message = 0;
            }
            // Test whether the master submitted a new job.
            int has_message = 0;
            MPI_Iprobe(master, vector_tag, MPI_COMM_WORLD, &has_message,
                       &status);
            if (has_message) {
                vector<int> v;
                v.resize(vector_size);
                MPI_Recv(&v[0], vector_size, MPI_INT, master, vector_tag,
                         MPI_COMM_WORLD, &status);

                vector<int> v_sorted = bubble_sort(v);
                MPI_Send(&v_sorted, vector_size, MPI_INT, master, vector_tag,
                         MPI_COMM_WORLD);

                ask_for_message = 1;
            }
            // Check for a 'suicide' request.
            MPI_Iprobe(master, kill_tag, MPI_COMM_WORLD, &kill_flag, &status);
        }
    } else {
        vector<vector<int>> vectors = load_dataset(number_vectors, vector_size);

        double begin = MPI_Wtime();

        // Store async requests received from workers.
        vector<MPI_Request> receive_requests(number_vectors);
        vector<vector<int>> ordered_vectors(number_vectors);

        int worker_request = 0;
        for (int i = 0; i < vectors.size(); i++) {
            vector<int> v = vectors.at(i);
            MPI_Recv(&worker_request, 1, MPI_INT, MPI_ANY_SOURCE,
                     request_vector_tag, MPI_COMM_WORLD, &status);
            // Send the vector to the worker.
            MPI_Send(&v, vector_size, MPI_INT, status.MPI_SOURCE, vector_tag,
                     MPI_COMM_WORLD);

            ordered_vectors[i].resize(vector_size);
            MPI_Irecv(&ordered_vectors[i][0], vector_size, MPI_INT,
                      status.MPI_SOURCE, vector_tag, MPI_COMM_WORLD,
                      &receive_requests[i]);
        }

        // Wait for all requests.
        for (int i = 0; i < vectors.size(); i++) {
            MPI_Wait(&receive_requests.at(i), &status);
        }

        // Kill all workers.
        int kill_value = 1;
        for (int i = 1; i < num_processes; i++) {
            MPI_Send(&kill_value, 1, MPI_INT, i, kill_tag, MPI_COMM_WORLD);
        }

        double end = MPI_Wtime();
        double total_time = end - begin;

        cout << "Number processes: " << num_processes << endl;
        cout << "Number vectors: " << number_vectors << endl;
        cout << "Vector size: " << vector_size << endl;
        cout << "Time sort (s): " << total_time << endl;
    }
    MPI_Finalize();

    return 0;
}

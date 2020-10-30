#include "dataset-generator.cpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <sstream>
#include <tuple>
#include <vector>

using namespace std;

vector<int> load_dataset(int vector_size) {
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    vector<int> vector_unsorted = dataset::get_dataset(vector_size);
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    double total_time =
        chrono::duration_cast<chrono::duration<double>>(end - begin).count();
    cout << "Time load dataset (s): " << total_time << endl;
    return vector_unsorted;
}

vector<int> interleaving(vector<int> vector_left, vector<int> vector_right) {
    vector<int> result;
    std::merge(vector_left.begin(), vector_left.end(), vector_right.begin(),
               vector_right.end(), std::back_inserter(result));
    return result;
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

template <typename T>
std::vector<T> slice(std::vector<T> const &v, int begin, int end) {
    std::vector<T> sliced(v.cbegin() + begin, v.cbegin() + end + 1);
    return sliced;
}

bool is_power_of_2(int x) { return x > 0 && !(x & (x - 1)); }

int main(int argc, char **argv) {
    int vector_size = atoi(argv[1]);
    int delta = atoi(argv[2]);

    MPI_Status status;
    int my_rank;
    int num_processes;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    int needed_processes =
        std::pow(2, 1 + std::floor(std::log2(vector_size / delta))) - 1;

    if (!is_power_of_2(vector_size / delta)) {
        cout << "Error: vector size divided by delta must be a power of 2."
             << endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
    } else if (num_processes != needed_processes) {
        cout << "Error: you need " << needed_processes
             << " processes, but only allocated " << num_processes << "."
             << endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    int parent_node = std::floor(std::abs((my_rank - 1) / 2));
    vector<int> sub_vector;
    vector<int> sub_vector_sorted;

    double begin;

    if (my_rank != 0) {
        MPI_Probe(parent_node, 0, MPI_COMM_WORLD, &status);
        int sub_vector_size;
        MPI_Get_count(&status, MPI_INT, &sub_vector_size);
        sub_vector.resize(sub_vector_size);
        MPI_Recv(&sub_vector[0], sub_vector_size, MPI_INT, parent_node, 0,
                 MPI_COMM_WORLD, &status);
    } else {
        sub_vector = load_dataset(vector_size);
        begin = MPI_Wtime();
    }

    if (sub_vector.size() <= delta) {
        sub_vector_sorted = bubble_sort(sub_vector);
    } else {
        int sub_vector_split_index = sub_vector.size() / 2;
        int left_node = (my_rank * 2) + 1;
        int right_node = (my_rank * 2) + 2;

        MPI_Send(&sub_vector[0], sub_vector_split_index, MPI_INT, left_node, 0,
                 MPI_COMM_WORLD);
        MPI_Send(&sub_vector[sub_vector_split_index], sub_vector_split_index,
                 MPI_INT, right_node, 0, MPI_COMM_WORLD);

        sub_vector_sorted.resize(sub_vector.size());

        vector<int> vector_left(sub_vector_split_index);
        vector<int> vector_right(sub_vector_split_index);
        MPI_Recv(&vector_left[0], sub_vector_split_index, MPI_INT, left_node, 0,
                 MPI_COMM_WORLD, &status);
        MPI_Recv(&vector_right[0], sub_vector_split_index, MPI_INT, right_node,
                 0, MPI_COMM_WORLD, &status);

        sub_vector_sorted = interleaving(vector_left, vector_right);
    }

    if (my_rank != 0) {
        MPI_Send(&sub_vector_sorted[0], sub_vector_sorted.size(), MPI_INT,
                 parent_node, 0, MPI_COMM_WORLD);
    } else {
        double end = MPI_Wtime();
        double total_time = end - begin;
        cout << "Vector size: " << vector_size << endl;
        cout << "Time sort (s): " << total_time << endl;
    }

    MPI_Finalize();

    return 0;
}

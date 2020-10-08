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

vector<vector<dataset::Point>> load_dataset(long long bucket_size,
                                            int number_buckets) {
    double begin = MPI_Wtime();

    vector<vector<dataset::Point>> buckets_points;
    for (int i = 0; i < number_buckets; i++) {
        buckets_points.push_back(dataset::get_dataset(bucket_size, 10));
    }
    double end = MPI_Wtime();
    double total_time = end - begin;
    cout << "Time load dataset (ms): " << total_time << endl;
    return buckets_points;
}

tuple<double, double, double> execute_lr(vector<dataset::Point> points) {
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    size_t n = points.size();
    long long x_sum = 0;
    long long y_sum = 0;
    long long x_squared_sum = 0;
    long long xy_sum = 0;

    for (long long i = 0; i < n; i++) {
        int x_aux = points.at(i).x;
        int y_aux = points.at(i).y;

        x_sum += x_aux;
        y_sum += y_aux;

        x_squared_sum += x_aux * x_aux;
        xy_sum += x_aux * y_aux;
    }

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    double total_time =
        chrono::duration_cast<chrono::milliseconds>(end - begin).count();

    double slope = ((double)(n * xy_sum - x_sum * y_sum)) /
                   ((double)(n * x_squared_sum - x_sum * x_sum));
    double intercept = ((double)(y_sum - slope * x_sum)) / n;

    return make_tuple(total_time, slope, intercept);
}

int main(int argc, char **argv) {
    int my_rank;
    int num_processes;
    int tag = 1;
    long long bucket_size = atoll(argv[1]);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    MPI_Datatype MPI_POINT_TYPE;
    const int count = 2;
    int block_lengths[count] = {1, 1};
    MPI_Aint displacements[count] = {offsetof(dataset::Point, x),
                                     offsetof(dataset::Point, y)};
    MPI_Datatype types[count] = {MPI_INT, MPI_INT};
    MPI_Type_create_struct(count, block_lengths, displacements, types,
                           &MPI_POINT_TYPE);
    MPI_Type_commit(&MPI_POINT_TYPE);

    if (my_rank != 0) {
        int master = 0;
        vector<dataset::Point> points;
        points.resize(bucket_size);

        MPI_Status status;
        MPI_Recv(&points[0], bucket_size, MPI_POINT_TYPE, master, tag,
                 MPI_COMM_WORLD, &status);

        cout << "Worker " << my_rank << " - " << points.size() << endl;
    } else {
        vector<vector<dataset::Point>> buckets_points =
            load_dataset(bucket_size, num_processes - 1);

        double begin = MPI_Wtime();
        for (int i = 0; i < buckets_points.size(); i++) {
            vector<dataset::Point> points = buckets_points.at(i);

            MPI_Send(&points[0], bucket_size, MPI_POINT_TYPE, i + 1, tag,
                     MPI_COMM_WORLD);
        }
        /* for (int i = 1; i < num_processes; i++) { */
        /*     int result; */
        /*     MPI_Recv(&result, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
         */
        /* } */
        double end = MPI_Wtime();
        double total_time = end - begin;
        cout << "Time linear regression (ms): " << total_time << endl;
    }
    MPI_Finalize();
    return 0;
}

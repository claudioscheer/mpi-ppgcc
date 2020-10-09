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

struct RegressionSubResults {
    unsigned long long int x_sum;
    unsigned long long int y_sum;
    unsigned long long int x_squared_sum;
    unsigned long long int xy_sum;
};

vector<vector<dataset::Point>> load_dataset(unsigned long long int bucket_size,
                                            unsigned int number_buckets) {
    double begin = MPI_Wtime();

    vector<vector<dataset::Point>> buckets_points;
    for (unsigned int i = 0; i < number_buckets; i++) {
        buckets_points.push_back(dataset::get_dataset(bucket_size));
    }
    double end = MPI_Wtime();
    double total_time = end - begin;
    cout << "Time load dataset (s): " << total_time << endl;
    return buckets_points;
}

RegressionSubResults execute_lr(vector<dataset::Point> points) {
    unsigned long long int x_sum = 0;
    unsigned long long int y_sum = 0;
    unsigned long long int x_squared_sum = 0;
    unsigned long long int xy_sum = 0;
    int n = (int)points.size();

    for (unsigned long long int i = 0; i < n; i++) {
        int x_aux = points.at(i).x;
        int y_aux = points.at(i).y;

        x_sum += x_aux;
        y_sum += y_aux;

        x_squared_sum += x_aux * x_aux;
        xy_sum += x_aux * y_aux;
    }

    return {
        .x_sum = x_sum,
        .y_sum = y_sum,
        .x_squared_sum = x_squared_sum,
        .xy_sum = xy_sum,
    };
}

int main(int argc, char **argv) {
    int my_rank;
    int num_processes;
    int tag = 1;
    unsigned long long int bucket_size = atoll(argv[1]);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    MPI_Datatype MPI_POINT_TYPE;
    int block_lengths_point[2] = {1, 1};
    MPI_Aint displacements_point[2] = {offsetof(dataset::Point, x),
                                       offsetof(dataset::Point, y)};
    MPI_Datatype types_point[2] = {MPI_INT, MPI_INT};
    MPI_Type_create_struct(2, block_lengths_point, displacements_point,
                           types_point, &MPI_POINT_TYPE);
    MPI_Type_commit(&MPI_POINT_TYPE);

    MPI_Datatype MPI_REGRESSION_SUB_RESULTS_TYPE;
    int block_lengths_regression_sub_results[4] = {1, 1, 1, 1};
    MPI_Aint displacements_regression_sub_results[4] = {
        offsetof(RegressionSubResults, x_sum),
        offsetof(RegressionSubResults, y_sum),
        offsetof(RegressionSubResults, x_squared_sum),
        offsetof(RegressionSubResults, xy_sum)};
    MPI_Datatype types_regression_sub_results[4] = {
        MPI_LONG_LONG_INT, MPI_LONG_LONG_INT, MPI_LONG_LONG_INT,
        MPI_LONG_LONG_INT};
    MPI_Type_create_struct(4, block_lengths_regression_sub_results,
                           displacements_regression_sub_results,
                           types_regression_sub_results,
                           &MPI_REGRESSION_SUB_RESULTS_TYPE);
    MPI_Type_commit(&MPI_REGRESSION_SUB_RESULTS_TYPE);

    MPI_Status status;

    if (my_rank != 0) {
        int master = 0;
        vector<dataset::Point> points;
        points.resize(bucket_size);

        MPI_Recv(&points[0], bucket_size, MPI_POINT_TYPE, master, tag,
                 MPI_COMM_WORLD, &status);

        RegressionSubResults sub_results = execute_lr(points);
        MPI_Send(&sub_results, 1, MPI_REGRESSION_SUB_RESULTS_TYPE, master, tag,
                 MPI_COMM_WORLD);
    } else {
        vector<vector<dataset::Point>> buckets_points =
            load_dataset(bucket_size, num_processes - 1);

        double begin = MPI_Wtime();
        for (int i = 0; i < buckets_points.size(); i++) {
            vector<dataset::Point> bucket_points = buckets_points.at(i);

            MPI_Send(&bucket_points[0], bucket_size, MPI_POINT_TYPE, i + 1, tag,
                     MPI_COMM_WORLD);
        }

        RegressionSubResults results = {
            .x_sum = 0,
            .y_sum = 0,
            .x_squared_sum = 0,
            .xy_sum = 0,
        };
        for (int i = 0; i < buckets_points.size(); i++) {
            RegressionSubResults sub_results;
            MPI_Recv(&sub_results, 1, MPI_REGRESSION_SUB_RESULTS_TYPE, i + 1,
                     tag, MPI_COMM_WORLD, &status);

            results.x_sum += sub_results.x_sum;
            results.y_sum += sub_results.y_sum;
            results.x_squared_sum += sub_results.x_squared_sum;
            results.xy_sum += sub_results.xy_sum;
        }
        double end = MPI_Wtime();
        double total_time = end - begin;

        unsigned long long int n = bucket_size * (num_processes - 1);

        double slope =
            ((double)(n * results.xy_sum - results.x_sum * results.y_sum)) /
            ((double)(n * results.x_squared_sum -
                      results.x_sum * results.x_sum));
        double intercept =
            ((double)(results.y_sum - slope * results.x_sum)) / n;
        cout << "Time linear regression (s): " << total_time << endl;
        cout << "Slope: " << slope << endl;
        cout << "Intercept: " << intercept << endl;
    }
    MPI_Finalize();

    return 0;
}

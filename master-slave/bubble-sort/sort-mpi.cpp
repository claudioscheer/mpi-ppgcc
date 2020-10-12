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

// Store the results from each worker.
struct RegressionSubResults {
    unsigned long long int x_sum;
    unsigned long long int y_sum;
    unsigned long long int x_squared_sum;
    unsigned long long int xy_sum;
};

vector<dataset::Point> load_dataset(unsigned long long int number_points) {
    double begin = MPI_Wtime();
    vector<dataset::Point> points = dataset::get_dataset(number_points);
    double end = MPI_Wtime();
    double total_time = end - begin;
    cout << "Time load dataset (s): " << total_time << endl;
    return points;
}

// Perform linear regression on the subvector.
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
    unsigned long long int number_points = atoll(argv[1]);
    unsigned long long int granularity = atoll(argv[2]);

    int vector_tag = 1;
    int kill_tag = 2;
    int request_vector_tag = 3;

    int number_grains = number_points / granularity;
    MPI_Status status;
    int my_rank;
    int num_processes;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    if ((number_points % granularity) > 0) {
        // This avoids the need to deal with the last elements of the array.
        cout << "Error: granularity must be a multiple of the number of points."
             << endl;
        MPI::COMM_WORLD.Abort(-1);
    }

    // Commit Point struct to MPI.
    MPI_Datatype MPI_POINT_TYPE;
    int block_lengths_point[2] = {1, 1};
    MPI_Aint displacements_point[2] = {offsetof(dataset::Point, x),
                                       offsetof(dataset::Point, y)};
    MPI_Datatype types_point[2] = {MPI_INT, MPI_INT};
    MPI_Type_create_struct(2, block_lengths_point, displacements_point,
                           types_point, &MPI_POINT_TYPE);
    MPI_Type_commit(&MPI_POINT_TYPE);

    // Commit RegressionSubResults struct to MPI.
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
                vector<dataset::Point> points;
                points.resize(granularity);
                MPI_Recv(&points[0], granularity, MPI_POINT_TYPE, master,
                         vector_tag, MPI_COMM_WORLD, &status);

                RegressionSubResults sub_results = execute_lr(points);
                MPI_Send(&sub_results, 1, MPI_REGRESSION_SUB_RESULTS_TYPE,
                         master, vector_tag, MPI_COMM_WORLD);

                ask_for_message = 1;
            }
            // Check for a 'suicide' request.
            MPI_Iprobe(master, kill_tag, MPI_COMM_WORLD, &kill_flag, &status);
        }
    } else {
        vector<dataset::Point> points = load_dataset(number_points);

        double begin = MPI_Wtime();

        // Store async requests received from workers.
        vector<MPI_Request> receive_requests(number_grains);
        vector<RegressionSubResults> regression_sub_results(number_grains);

        int grain = 0;
        while (number_points > (grain * granularity)) {
            // Test if a worker is asking for a job.
            int has_worker_request = 0;
            MPI_Iprobe(MPI_ANY_SOURCE, request_vector_tag, MPI_COMM_WORLD,
                       &has_worker_request, &status);
            if (has_worker_request) {
                // Send the next elements from the dataset to the worker.
                MPI_Send(&points[(grain * granularity)], granularity,
                         MPI_POINT_TYPE, status.MPI_SOURCE, vector_tag,
                         MPI_COMM_WORLD);
                MPI_Irecv(&regression_sub_results[grain], 1,
                          MPI_REGRESSION_SUB_RESULTS_TYPE, status.MPI_SOURCE,
                          vector_tag, MPI_COMM_WORLD, &receive_requests[grain]);
                grain++;
            }
        }

        RegressionSubResults results = {
            .x_sum = 0,
            .y_sum = 0,
            .x_squared_sum = 0,
            .xy_sum = 0,
        };
        // Collect the results of all workers.
        for (int i = 0; i < number_grains; i++) {
            MPI_Wait(&receive_requests.at(i), &status);
            RegressionSubResults sub_results = regression_sub_results.at(i);
            results.x_sum += sub_results.x_sum;
            results.y_sum += sub_results.y_sum;
            results.x_squared_sum += sub_results.x_squared_sum;
            results.xy_sum += sub_results.xy_sum;
        }

        // Kill all workers.
        int kill_value = 1;
        for (int i = 1; i < num_processes; i++) {
            MPI_Send(&kill_value, 1, MPI_INT, i, kill_tag, MPI_COMM_WORLD);
        }

        double end = MPI_Wtime();
        double total_time = end - begin;

        double slope = ((double)(number_points * results.xy_sum -
                                 results.x_sum * results.y_sum)) /
                       ((double)(number_points * results.x_squared_sum -
                                 results.x_sum * results.x_sum));
        double intercept =
            ((double)(results.y_sum - slope * results.x_sum)) / number_points;
        cout << "Time linear regression (s): " << total_time << endl;
        cout << "Slope: " << slope << endl;
        cout << "Intercept: " << intercept << endl;
    }
    MPI_Finalize();

    return 0;
}

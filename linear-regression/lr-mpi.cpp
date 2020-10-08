#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <sstream>
#include <tuple>
#include <vector>

using namespace std;

struct Point {
    int x;
    int y;
};

string get_dataset_name(int n) {
    if (n == 1) {
        return "lr_dataset_1.txt";
    } else if (n == 2) {
        return "lr_dataset_2.txt";
    } else if (n == 3) {
        return "lr_dataset_3.txt";
    }
    throw runtime_error("Error: unable to load the dataset.");
}

vector<int> create_buckets(long long n, int number_buckets) {
    int minimal_bucket_size = n / number_buckets;
    vector<int> b(number_buckets, minimal_bucket_size);
    fill(b.begin(), b.begin() + (n % number_buckets), minimal_bucket_size + 1);
    return b;
}

vector<vector<Point>> load_dataset(string dataset_name, int number_buckets) {
    double begin = MPI_Wtime();

    vector<vector<Point>> buckets_points;
    FILE *file = fopen(dataset_name.c_str(), "rt");
    if (file != NULL) {
        long long number_points = 0;
        fscanf(file, "%lld", &number_points);

        vector<int> buckets_size =
            create_buckets(number_points, number_buckets);

        long long x = 0;
        long long y = 0;
        for (int i = 0; i < buckets_size.size(); i++) {
            int bucket_size = buckets_size.at(i);
            vector<Point> points;
            for (int j = 0; j < bucket_size; j++) {
                fscanf(file, "%lld %lld", &x, &y);
                Point p;
                p.x = x;
                p.y = y;
                points.push_back(p);
            }
            buckets_points.push_back(points);
        }

        fclose(file);
    } else {
        runtime_error("Error: cannot open the dataset file.");
    }

    double end = MPI_Wtime();
    double total_time = end - begin;
    cout << "Time load dataset (ms): " << total_time << endl;

    return buckets_points;
}

tuple<double, double, double> execute_lr(vector<Point> points) {
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

/*
 * TO DO:
 * - Use array instead of struct;
 */
int main(int argc, char **argv) {
    int my_rank;
    int num_processes;
    int tag = 1;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    MPI_Datatype MPI_POINT_TYPE;
    const int count = 2;
    int block_lengths[count] = {1, 1};
    MPI_Aint displacements[count] = {offsetof(Point, x), offsetof(Point, y)};
    MPI_Datatype types[count] = {MPI_INT, MPI_INT};
    MPI_Type_create_struct(count, block_lengths, displacements, types,
                           &MPI_POINT_TYPE);
    MPI_Type_commit(&MPI_POINT_TYPE);

    if (my_rank != 0) {
        int master = 0;
        vector<Point> points;
        points.resize(6);

        MPI_Status status;
        MPI_Recv(&points[0], 6, MPI_POINT_TYPE, master, tag, MPI_COMM_WORLD,
                 &status);

        cout << "Worker " << my_rank << " - " << points.size() << endl;
    } else {
        long long n = atoll(argv[1]);
        string dataset_name = get_dataset_name(n);
        vector<vector<Point>> buckets_points =
            load_dataset(dataset_name, num_processes - 1);

        double begin = MPI_Wtime();
        for (int i = 0; i < buckets_points.size(); i++) {
            vector<Point> points = buckets_points.at(i);

            MPI_Send(&points[0], points.size(), MPI_POINT_TYPE, i + 1, tag,
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

#include "dataset-generator.cpp"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

using namespace std;

vector<dataset::Point> load_dataset(long long bucket_size, int number_buckets) {
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    vector<dataset::Point> points =
        dataset::get_dataset(bucket_size * number_buckets);

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    double total_time =
        chrono::duration_cast<chrono::milliseconds>(end - begin).count();
    cout << "Time load dataset (ms): " << total_time << endl;
    return points;
}

tuple<double, double, double> execute_lr(vector<dataset::Point> points) {
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    long long x_sum = 0;
    long long y_sum = 0;
    long long x_squared_sum = 0;
    long long xy_sum = 0;
    int n = (int)points.size();

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
    cout << slope << endl;
    double intercept = ((double)(y_sum - slope * x_sum)) / n;

    return make_tuple(total_time, slope, intercept);
}

int main(int argc, char **argv) {
    long long bucket_size = atoll(argv[1]);
    int number_buckets = atoll(argv[2]);
    vector<dataset::Point> points = load_dataset(bucket_size, number_buckets);
    tuple<double, double, double> results = execute_lr(points);

    double total_time = get<0>(results);
    double slope = get<1>(results);
    double intercept = get<2>(results);
    cout << "Time linear regression (ms): " << total_time << endl;
    cout << "Slope: " << slope << endl;
    cout << "Intercept: " << intercept << endl;

    return 0;
}

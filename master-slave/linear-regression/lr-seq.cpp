#include "dataset-generator.cpp"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

using namespace std;

vector<dataset::Point> load_dataset(unsigned long long int number_points) {
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    vector<dataset::Point> points = dataset::get_dataset(number_points);
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    double total_time = chrono::duration<double>(end - begin).count();
    cout << "Time load dataset (s): " << total_time << endl;
    return points;
}

tuple<double, double, double> execute_lr(vector<dataset::Point> points) {
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

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

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    double total_time = chrono::duration<double>(end - begin).count();

    double slope = ((double)(n * xy_sum - x_sum * y_sum)) /
                   ((double)(n * x_squared_sum - x_sum * x_sum));
    double intercept = ((double)(y_sum - slope * x_sum)) / n;

    return make_tuple(total_time, slope, intercept);
}

int main(int argc, char **argv) {
    unsigned long long int number_points = atoll(argv[1]);
    vector<dataset::Point> points = load_dataset(number_points);
    tuple<double, double, double> results = execute_lr(points);

    double total_time = get<0>(results);
    double slope = get<1>(results);
    double intercept = get<2>(results);
    cout << "Time linear regression (s): " << total_time << endl;
    cout << "Slope: " << slope << endl;
    cout << "Intercept: " << intercept << endl;

    return 0;
}

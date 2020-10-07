#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
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

vector<Point> load_dataset(string dataset_name) {
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    vector<Point> points;
    FILE *file = fopen(dataset_name.c_str(), "rt");
    if (file != NULL) {
        long long number_of_points = 0;
        fscanf(file, "%lld", &number_of_points);
        long long x = 0;
        long long y = 0;

        for (int i = 0; i < number_of_points; i++) {
            fscanf(file, "%lld %lld", &x, &y);
            Point p;
            p.x = x;
            p.y = y;
            points.push_back(p);
        }

        fclose(file);
    } else {
        runtime_error("Error: cannot open the dataset file.");
    }

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    double total_time =
        chrono::duration_cast<chrono::milliseconds>(end - begin).count();

    cout << "Time load dataset (ms): " << total_time << endl;

    return points;
}

tuple<double, double> execute_lr(vector<Point> points) {
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

    double slope = ((double)(n * xy_sum - x_sum * y_sum)) /
                   ((double)(n * x_squared_sum - x_sum * x_sum));
    double intercept = ((double)(y_sum - slope * x_sum)) / n;

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    double total_time =
        chrono::duration_cast<chrono::milliseconds>(end - begin).count();

    cout << "Time linear regression (ms): " << total_time << endl;

    return make_tuple(slope, intercept);
}

int main(int argc, char **argv) {
    long long n = atoll(argv[1]);
    string dataset_name = get_dataset_name(n);
    vector<Point> points = load_dataset(dataset_name);
    tuple<double, double> slope_intercept = execute_lr(points);
    return 0;
}

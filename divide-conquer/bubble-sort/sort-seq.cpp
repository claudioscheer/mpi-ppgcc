#include "dataset-generator.cpp"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
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
    int vector_size = atoi(argv[1]);
    vector<int> vector_unsorted = load_dataset(vector_size);

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    vector<int> v_sorted = bubble_sort(vector_unsorted);
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    double total_time =
        chrono::duration_cast<chrono::duration<double>>(end - begin).count();

    cout << "Vector size: " << vector_size << endl;
    cout << "Time sort (s): " << total_time << endl;
    return 0;
}

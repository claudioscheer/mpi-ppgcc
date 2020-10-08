#include <iostream>
#include <vector>

using namespace std;

namespace dataset {
struct Point {
    long long x;
    long long y;
};

const int MOD_VALUE_X = 11;
const int MOD_VALUE_Y = 2;

vector<Point> get_dataset(long long size) {
    vector<Point> points;
    for (long long i = 0; i < size; i++) {
        Point p = {.x = i % MOD_VALUE_X, .y = i % MOD_VALUE_Y};
        points.push_back(p);
    }
    return points;
}
} // namespace dataset

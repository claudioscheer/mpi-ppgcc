#include <iostream>
#include <vector>

using namespace std;

namespace dataset {
struct Point {
    int x;
    int y;
};

const int MOD_VALUE_X = 99;
const int MOD_VALUE_Y = 33;

vector<Point> get_dataset(unsigned long long int size) {
    vector<Point> points;
    for (unsigned long long int i = 0; i < size; i++) {
        Point p = {.x = (int)i % MOD_VALUE_X, .y = (int)i % MOD_VALUE_Y};
        points.push_back(p);
    }
    return points;
}
} // namespace dataset

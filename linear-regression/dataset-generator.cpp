#include <iostream>
#include <vector>

namespace dataset {

using namespace std;
struct Point {
    int x;
    int y;
};

vector<Point> get_dataset(long long size, int values_interval) {
    vector<Point> points;
    for (long long i = 0; i < size; i++) {
        Point p;
        p.x = rand() % values_interval;
        p.y = rand() % values_interval;
        points.push_back(p);
    }
    return points;
}
} // namespace dataset

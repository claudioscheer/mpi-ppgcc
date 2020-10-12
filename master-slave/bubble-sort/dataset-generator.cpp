#include <iostream>
#include <vector>

using namespace std;

namespace dataset {
vector<int> get_vector(int vector_size) {
    vector<int> v;
    for (int i = 0; i < vector_size; i++) {
        v.push_back(vector_size - i);
    }
    return v;
}

vector<vector<int>> get_dataset(int number_vectors, int vector_size) {
    vector<vector<int>> vectors;
    vector<int> v = get_vector(vector_size);
    for (int i = 0; i < number_vectors; i++) {
        vectors.push_back(v);
    }
    return vectors;
}
} // namespace dataset

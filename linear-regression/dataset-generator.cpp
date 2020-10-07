#include <fstream>
#include <iostream>

using namespace std;

void create_dataset(long long int values_interval,
                    long long int number_of_points, string file_name) {
    ofstream file;
    file.open(file_name);

    if (file.is_open()) {
        file << number_of_points << "\n";
        for (long long int i = 0; i < number_of_points; i++) {
            file << rand() % values_interval << " " << rand() % values_interval
                 << "\n";
        }
        file.close();
    } else {
        runtime_error("Error: cannot open the dataset file.");
    }
}

int main(int argc, char **argv) {
    long long int n_points_dataset_1 = atoll(argv[1]);
    long long int n_points_dataset_2 = atoll(argv[2]);
    long long int n_points_dataset_3 = atoll(argv[3]);
    long long int values_interval = atoll(argv[4]);

    create_dataset(values_interval, n_points_dataset_1, "lr_dataset_1.txt");
    create_dataset(values_interval, n_points_dataset_2, "lr_dataset_2.txt");
    create_dataset(values_interval, n_points_dataset_3, "lr_dataset_3.txt");
}

// This program prints from multiple threads using OpenMP

#include <iostream>
#include <omp.h>

using namespace std;

int main() {
    int i = 0;
#pragma omp parallel //num_threads(1)
    {
#pragma omp critical
        {
            ++i;
            cout << "Printing from thread: " << omp_get_thread_num() << '\n';
        }
    }   
    cout << "i = " << i << endl;

    return 0;
}
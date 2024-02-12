#include <iostream>
#include <fstream>
#include <math.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>

using namespace std;

mutex mut;

typedef chrono::microseconds MSEC;
typedef chrono::high_resolution_clock HRC;

// A function to check if a number is prime or not
bool checkPrime(long num) {
    if (num <= 1)
        return false;

    // num is not a prime if it has any other factors
    for (long i = 2; i * i <= num; i++) {
        if (num % i == 0)
            return false;
    }

    // num has only two factors: 1 and itself
    return true;
}

void thread_function(int thread_no, int m, long N, ofstream& outputFile) {
    for (long num = (thread_no + 1); num <= N; num += m) { // increment by m as per the algorithm
        // Output the number if it is prime
        if (checkPrime(num)) {
            lock_guard<mutex> guard(mut);
            outputFile << num << " ";
        } 
    }
}

int main() {
    // Read input from a file
    ifstream inputFile;
    inputFile.open("inp-params.txt");
    int n, m;

    if (!inputFile.is_open()) {
        cout << "Error while opening file" << endl;
        exit(1); 
    }
    inputFile >> n >> m;
    cout << "n = " << n << " ; m = " << m << endl;

    // Start the timer
    auto start = HRC::now();

    // Create an output file
    ofstream outputFile ("Primes-SAM1.txt", ios::out);

    // Create a vector of threads and set number of threads to m
    vector<thread> threads (m);

    // N = 10^n is the limit
    long N = (long)pow(10, n);

    // Execute the for loop in parallel
    for (int i = 0; i < m; i++) {
        threads[i] = thread(thread_function, i, m, N, ref(outputFile));
    }

    // Join the threads
    for (int i = 0; i < m; i++) 
        threads[i].join();

    // Check the execution time
    auto end = HRC::now();
    auto exec_time = chrono::duration_cast<MSEC>(end - start).count();
    cout << "Execution time: " << exec_time << endl;

    outputFile.close();
    inputFile.close();
}
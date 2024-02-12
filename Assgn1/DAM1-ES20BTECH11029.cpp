#include <iostream>
#include <fstream>
#include <math.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>

using namespace std;

typedef chrono::microseconds MSEC;
typedef chrono::high_resolution_clock HRC;

mutex mut;

// A class Counter which has an attribute value and a function get and increment it
class Counter {
    long value;

    public:
    Counter(){
        value = 1;
    }

    long getAndIncrement() {
        // Critical section: 
        // At a time, only one thread should enter this section to increment value variable in counter object
        lock_guard<mutex> guard(mut);
        return value++;
    }
};

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

void thread_function(Counter& counter, ofstream& outputFile, long N, int id) {
    long num;
    do {
        num = counter.getAndIncrement();
    
        if (checkPrime(num)) {
            lock_guard<mutex> guard(mut);
            outputFile << num << " ";
        }

    } while (num <= N);
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

    // Create a Counter object
    Counter counter = Counter();
    
    // Create an output file
    ofstream outputFile ("Primes-DAM1.txt", ios::out);

    // Start the timer
    auto start = HRC::now();

    // Create a vector of threads and set number of threads to m
    vector<thread> threads (m);

    // N = 10^n is the limit
    long N = (long)pow(10, n);

    // Execute the for loop in parallel
    for (int i = 0; i < m; i++) {
        threads[i] = thread(thread_function, ref(counter), ref(outputFile), N, i);
    }

    // join the threads
    for (int i = 0; i < m; i++) 
        threads[i].join();
    
    // Check the execution time
    auto end = HRC::now();
    auto exec_time = chrono::duration_cast<MSEC>(end - start).count();
    cout << "Execution time: " << exec_time << endl;

    outputFile.close();
    inputFile.close();
}
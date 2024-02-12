#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <thread>
#include <random>
#include <chrono>
#include <atomic>
#include <limits.h>
#include <optional>

using namespace std;

typedef chrono::milliseconds MSEC;
typedef chrono::microseconds MCROSEC;
typedef chrono::nanoseconds NSEC;
typedef chrono::high_resolution_clock HRC;

template <typename T>
class NLQueue {
    atomic<T>* items;
    atomic<int> tail;
    const int CAPACITY = INT_MAX;

    public:
    NLQueue() {
        items = new atomic<T>[CAPACITY];
        tail.store(0);
    }

    void enq(T x) {
        int i = tail.fetch_add(1);
        items[i].store(x);
    }

    T deq() {
        while (true) {
            int range = tail.load();
            for (int i=0; i<range; i++) {
                T value = items[i].exchange(0);
                if (value != 0) 
                    return value;
            }
        }
    }

    ~NLQueue() {
        delete[] items;
    }
};

int n, numOps;
double rndLt, lambda;
long *thrTimes;
atomic<long> totalEnqTime = 0, totalEnqOps = 0;
atomic<long> totalDeqTime = 0, totalDeqOps = 0;

void testThread(NLQueue<int> &qObj, int thread_no) {
    long sleepTime;
    HRC::time_point startTime, endTime;

    // Create random number generators
    random_device rd;
    default_random_engine unifRandObj(rd());
    default_random_engine expRandObj(lambda);

    // Create distributions
    std::uniform_real_distribution<double> unifDistr(0.0, 1.0); 
    std::exponential_distribution<double> expDistr(lambda); 

    int v;
    for (int i=0; i<numOps; i++) {
        double p = unifDistr(unifRandObj);

        if (p < rndLt) { // Enqueue operation
            v = rand() % 100 + 1;
            startTime = HRC::now(); 
            qObj.enq(v);
            endTime = HRC::now();
            totalEnqTime += chrono::duration_cast<NSEC>(endTime - startTime).count();
            totalEnqOps += 1;
        } 
        else { // Dequeue operation
            startTime = HRC::now(); 
            v = qObj.deq();
            endTime = HRC::now(); 
            totalDeqTime += chrono::duration_cast<NSEC>(endTime - startTime).count();
            totalDeqOps += 1;
        }

        // Store the execution time of the current operation
        thrTimes[thread_no] += chrono::duration_cast<NSEC>(endTime - startTime).count();
        
        // Simulate performing some other tasks using sleep
        sleepTime = (long)(expDistr(expRandObj) * 1000);
        this_thread::sleep_for(MSEC(sleepTime));
    }
}

void computeStats() {
    double avgEnqTime = (double) totalEnqTime / totalEnqOps;
    double avgDeqTime = (double) totalDeqTime / totalDeqOps;

    long totalTime = 0;
    for (int i=0; i<n; i++) {
        totalTime += thrTimes[i];
    }
    
    double avgTime =  (double) totalTime / (numOps * n);        

    ofstream output_file("NLQ-out.txt");
    output_file << avgEnqTime << " " << avgDeqTime << " " << avgTime << endl;
    output_file.close();
}

int main() {
    // Read input file
    ifstream input_file;
    input_file.open("inp-params.txt");      

    while(!input_file.is_open()) {
        cout << "Error while opening file" << endl;
        exit(1);
    }

    input_file >> n >> numOps >> rndLt >> lambda;

    NLQueue<int> qObj = NLQueue<int>(); 

    vector<thread> threads(n);
    thrTimes = new long[n];
    for (int i=0; i<n; i++) {
        threads[i] = thread(testThread, ref(qObj), i);
    }

    // Join all threads
    for (int i=0; i<n; i++) 
        threads[i].join();
    
    // Compute various statistics
    computeStats();

    // Close the input file
    input_file.close();
}
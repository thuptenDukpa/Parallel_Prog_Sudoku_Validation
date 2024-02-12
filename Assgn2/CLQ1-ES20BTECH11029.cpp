#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>
#include <mutex>
#include <thread>

using namespace std;

typedef chrono::milliseconds MSEC;
typedef chrono::microseconds MCROSEC;
typedef chrono::nanoseconds NSEC;
typedef chrono::high_resolution_clock HRC;

template <typename T>
class CLQueue {
    int head, tail;
    vector<T> items;
    mutex lock;

    public:
    CLQueue(int capacity) : head(0), tail(0), items(capacity){ }

    void enq(T x) {
        lock.lock();
        try {
            if (tail - head == items.size())
                throw "Full Exception";
            items[tail % items.size()] = x;
            tail++;
        } catch (const char *excp) {
        } 
        lock.unlock();
    }   

    T deq() {
        lock.lock();
        T x;
        try {
            if (tail == head)
                throw "Empty Exception";
            
            x = items[head % items.size()];
            head++;

        } catch (const char *excp) {
        }
        lock.unlock();
        return x;
    }
};

int n, numOps;
double rndLt, lambda;
long *thrTimes;
atomic<long> totalEnqTime = 0, totalEnqOps = 0;
atomic<long> totalDeqTime = 0, totalDeqOps = 0;

void testThread(CLQueue<int> &qObj, int thread_no) {
    double unifVal;
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
            v = rand() % 100;
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
        this_thread::sleep_for(chrono::milliseconds(sleepTime));
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

    ofstream output_file("CLQ-out.txt");
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

    int capacity = numOps * n;
    CLQueue<int> qObj = CLQueue<int>(capacity); 

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
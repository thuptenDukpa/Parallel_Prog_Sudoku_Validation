#include <iostream>
#include <atomic>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <mutex>

using namespace std;

typedef chrono::milliseconds MSEC;
typedef chrono::microseconds MCROSEC;
typedef chrono::nanoseconds NSEC;
typedef chrono::high_resolution_clock HRC;

// Base class which will be inherited by CLHLock
class Lock{
    public: // virtual functions
        virtual void lock(int thread_id) = 0;
        virtual void unlock(int thread_id) = 0;
};

struct QNode {
    atomic<bool> *locked = new atomic<bool>(false); 
};

// Input Variables
int n, k, lambda1, lambda2;

mutex mutx;
atomic<long> totalCSEntryTime = 0;

// Output file
ofstream output_file("CLH-LogFile.txt");

class CLHLock: public Lock {
    atomic<QNode> *tail;
    QNode *myPred;
    QNode *myNode;

    public:
    CLHLock(int n_threads) {
        tail = new atomic<QNode>(QNode());
        myPred = new QNode[n_threads];
        myNode = new QNode[n_threads];
    }

    void lock(int thread_id) {
        QNode qnode = myNode[thread_id];
        qnode.locked->store(true);

        QNode pred = tail->exchange(qnode);
        myPred[thread_id] = pred;

        while (pred.locked->load()) {
        }
    }

    void unlock(int thread_id) {
        QNode qnode = myNode[thread_id];
        qnode.locked->store(false);
        myNode[thread_id] = myPred[thread_id];
    }
};

void testCS(Lock *Test, int id) {
    time_t current_time;
    tm time_info;
    int curr_hour, curr_min, curr_sec;

    default_random_engine expRandObj1(lambda1);
    exponential_distribution<double> expDistr1(lambda1); 

    default_random_engine expRandObj2(lambda2);
    exponential_distribution<double> expDistr2(lambda2); 

    for (int i = 0; i < k; i++) { 
        // Log entry request time
        HRC::time_point reqEnterTime = HRC::now();
        current_time = chrono::system_clock::to_time_t(reqEnterTime);

        // Convert the current time to a struct tm
        time_info = *std::localtime(&current_time);

        // Using lock to ensure mutual exclusion while logging this output
        mutx.lock(); 
        output_file << i << "th CS Entry Request at " << time_info.tm_hour << ":" << time_info.tm_min << ":" << time_info.tm_sec << " by thread " << id << endl;
        mutx.unlock();

        /* Invoke lock() from CLH Lock */ 
        Test->lock(id);

        // Log actual entry time
        HRC::time_point actEnterTime = HRC::now();
        current_time = chrono::system_clock::to_time_t(reqEnterTime);
        time_info = *std::localtime(&current_time);
        
        output_file << i << "th CS Entry at " << time_info.tm_hour << ":" << time_info.tm_min << ":" << time_info.tm_sec << " by thread " << id << endl;

        // Track the time taken for thread to enter CS from time of request
        totalCSEntryTime += chrono::duration_cast<MSEC>(actEnterTime - reqEnterTime).count();

        // Sleep (t1)
        double s1 = expDistr1(expRandObj1);
        chrono::duration<double> duration(s1 / 1000); // since s1 is in seconds, we want to sleep in millisec
        this_thread::sleep_for(duration);

        // Log exit request time
        HRC::time_point reqExitTime = HRC::now();
        current_time = chrono::system_clock::to_time_t(reqExitTime);
        time_info = *std::localtime(&current_time);

        output_file << i << "th CS Exit request at " << time_info.tm_hour << ":" << time_info.tm_min << ":" << time_info.tm_sec << " by thread " << id << endl;

        /* Invoke unlock() from CLH Lock */ 
        Test->unlock(id);

        // Log actual exit request time
        HRC::time_point actExitTime = HRC::now();
        current_time = chrono::system_clock::to_time_t(actExitTime);
        time_info = *std::localtime(&current_time);

        mutx.lock();
        output_file << i << "th CS Exit at " << time_info.tm_hour << ":" << time_info.tm_min << ":" << time_info.tm_sec << " by thread " << id << endl;
        mutx.unlock();

        // Sleep (t2)
        double s2 = expDistr2(expRandObj2);
        chrono::duration<double> duration2(s2 / 1000);
        this_thread::sleep_for(duration2);
    }
}

int main() {
    // Read input file
    ifstream input_file;
    input_file.open("inp-params.txt");      

    while(!input_file.is_open()) {
        cout << "Error while opening file" << endl;
        exit(1);
    }

    input_file >> n >> k >> lambda1 >> lambda2;
    cout << "n = " << n << ", k = " << k << endl;

    // Close the input file
    input_file.close();

    cout << "CLH Lock Output: " << endl;
    output_file << "CLH Lock Output:" << endl;

    // Start time
    HRC::time_point startTime = HRC::now();
    time_t current_time = chrono::system_clock::to_time_t(startTime);

    // Convert the current time to a struct tm
    tm time_info = *localtime(&current_time);

    // Extract the current hour and minute
    cout << "The start time is " << time_info.tm_hour << ":" << time_info.tm_min << ":" << time_info.tm_sec << endl;

    // Declaring a lock object which is accessible from all threads
    CLHLock *Test = new CLHLock(n);

    // Creating n threads
    vector<thread> threads(n);
    for (int i = 0; i < n; i++) 
        threads[i] = thread(&testCS, Test, i);
    
    // Join all Threads
    for (int i = 0; i < n; i++) 
        threads[i].join();

    // End time
    HRC::time_point endTime = HRC::now();
    current_time = chrono::system_clock::to_time_t(endTime);
    time_info = *std::localtime(&current_time);

    cout << "The end time is " << time_info.tm_hour << ":" << time_info.tm_min << ":" << time_info.tm_sec << endl;

    // Calculate overall time in milliseconds
    long overallTime = chrono::duration_cast<MSEC>(endTime - startTime).count();
    cout << "Total time taken(in millisec): " << overallTime << endl;

    // Calculate Throughput 
    double throughput = (double)(k * n * 1000) / overallTime; // multiplied by 1000 as time is in milliseconds
    cout << "Throughput: " << throughput << " operations per second" << endl;

    // Calculate Average Entry Time
    double avgEntryTime = (double)totalCSEntryTime / (k * n);
    cout << "Average Entry time: " << avgEntryTime << " millisecs" << endl;
}


#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_map>

using namespace std;

typedef chrono::milliseconds MSEC;
typedef chrono::microseconds MCROSEC;
typedef chrono::nanoseconds NSEC;
typedef chrono::high_resolution_clock HRC;

unordered_map<thread::id, int> thread_id_map;
mutex mutx;

template <typename T>
class StampedValue {
    public:
    long stamp;
    T value;

    StampedValue (T init=0) {
        stamp = 0;
        value = init;
    }

    StampedValue(long ts, T v) {
        stamp = ts;
        value = v;
    }

    static StampedValue max(StampedValue x, StampedValue y) {
        if (x.stamp > y.stamp) {
            return x;
        } else {
            return y;
        }
    }
};

template <typename T>
class AtomicMRMWRegister {
    private:
    vector<atomic<StampedValue<T>>> a_table; // array of atomic MRSW registers
    
    public:
    AtomicMRMWRegister(int capacity, T init) {
        a_table = vector<atomic<StampedValue<T>>>(capacity);
        StampedValue<T> value = StampedValue<T>(init);
        for (int j = 0; j < a_table.size(); j++) {
            a_table[j] = value;
        }
    }

    void write(T value) {
        int me = thread_id_map[this_thread::get_id()];
        StampedValue<T> max(-1,0);

        for (int i = 0; i < a_table.size(); i++) {
            max = StampedValue<T>::max(max, a_table[i]);
        }
        a_table[me] = StampedValue(max.stamp + 1, value);
    }

    T read() {
        StampedValue<T> max(-1,0);
        for (int i = 0; i < a_table.size(); i++) {
            max = StampedValue<T>::max(max, a_table[i]);
        }
        return max.value;
    }
};

int capacity, numOps;
double lambda;
atomic<long> totalTime;
ofstream output_file("LogFile.txt");

template <typename T>
void testAtomic(AtomicMRMWRegister<T> &shVar, int thread_no) {
    int lVar; // local variable
    int id = thread_no;

    // thread_id_map maps the global unique id of thread to range from 0 to capacity-1
    thread_id_map[this_thread::get_id()] = id;

    // Stores system time before and after operation
    HRC::time_point reqTime, complTime;

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
        string action = (p > 0.5)? "read":"write";

        reqTime = HRC::now();
        
        mutx.lock();
        output_file << i << "th action requested at " << chrono::duration_cast<NSEC>(reqTime.time_since_epoch()).count()<< " by thread " << id << endl;
        mutx.unlock();

        if (action == "read") { // read operation
            lVar = shVar.read();

            mutx.lock();
            output_file << "Value read: " << lVar << endl;
            mutx.unlock();
        } 
        else { // write operation
            lVar = numOps * id; // value written by each thread is unique

            shVar.write(lVar);

            mutx.lock();
            output_file << "Value written: " << lVar << endl;
            mutx.unlock();
        }
        

        complTime = HRC::now(); 

        mutx.lock();
        output_file << i << "th action " << action << " completed at " << chrono::duration_cast<NSEC>(complTime.time_since_epoch()).count() << " by thread " << id << endl;
        mutx.unlock();

        // Simulate performing some other tasks using sleep
        long t1 = (long)(expDistr(expRandObj) * 10);
        this_thread::sleep_for(chrono::milliseconds(t1));

        // Store the execution time of the current operation
        totalTime += chrono::duration_cast<NSEC>(complTime - reqTime).count();  
    }
}

template <typename T>
void testAtomic2(atomic<T> &shVar, int thread_no) {
    int lVar; // local variable
    int id = thread_no;

    // thread_id_map maps the global unique id of thread to range from 0 to capacity-1
    thread_id_map[this_thread::get_id()] = id;

    // Stores system time before and after operation
    HRC::time_point reqTime, complTime;

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
        string action = (p > 0.5)? "read":"write";

        reqTime = HRC::now();

        mutx.lock();
        output_file << i << "th action requested at " << chrono::duration_cast<NSEC>(reqTime.time_since_epoch()).count()<< " by thread " << id << endl;
        mutx.unlock();

        if (action == "read") { // read operation
            lVar = shVar.load();

            mutx.lock();
            output_file << "Value read: " << lVar << endl;
            mutx.unlock();
        } 
        else { // write operation
            lVar = numOps * id; // value written by each thread is unique

            shVar.store(lVar);

            mutx.lock();
            output_file << "Value written: " << lVar << endl;
            mutx.unlock();
        }
        

        complTime = HRC::now(); 

        mutx.lock();
        output_file << i << "th action " << action << " completed at " << chrono::duration_cast<NSEC>(complTime.time_since_epoch()).count() << " by thread " << id << endl;
        mutx.unlock();

        // Simulate performing some other tasks using sleep
        long t1 = (long)(expDistr(expRandObj) * 10);
        this_thread::sleep_for(chrono::milliseconds(t1));

        // Store the execution time of the current operation
        totalTime += chrono::duration_cast<NSEC>(complTime - reqTime).count();  
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

    input_file >> capacity >> numOps >> lambda;

    // atomic<int> shVar = 0;
    AtomicMRMWRegister<int> shVar = AtomicMRMWRegister<int>(capacity, 0);

    vector<thread> threads(capacity);
    for (int i=0; i<capacity; i++) {
        // threads[i] = thread(testAtomic2<int>, ref(shVar), i);
        threads[i] = thread(testAtomic<int>, ref(shVar), i);  // i is threadID
    }

    // Join all threads
    for (int i=0; i<capacity; i++) 
        threads[i].join();

    // Close the input file
    input_file.close();

    // Write the output file
    long averageTime = totalTime / numOps;
    output_file << "Average Time: " << averageTime << endl;

    // Close the output file
    output_file.close();
}
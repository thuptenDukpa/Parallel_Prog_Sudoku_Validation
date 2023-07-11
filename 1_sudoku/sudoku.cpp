#include <iostream>
#include <omp.h>
#include <chrono>
#include <fstream>
#include <vector>
#include <unordered_set>

using namespace std;

typedef chrono::milliseconds MSEC;
typedef chrono::high_resolution_clock HRC;

int main() {
    // Start the timer
    auto start = HRC::now();

    // Read the inputs
    ifstream inputFile("../input.txt");
    int k, n;
    inputFile >> k >> n;
    cout << "k = " << k << " n = " << n << endl;

    vector<vector<int>> matrix(n, vector<int>(n));  
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            inputFile >> matrix[i][j];  
        }
    }
    inputFile.close();

    // Check Sudoku conditions
    bool is_valid = true;

    // Check: Each row contains unique values from 1 - N.
    for (int row = 0; row < n; row++) {
        unordered_set<int> set;
        for (int col = 0; col < n; col++) {
            // Check range of numbers
            if (matrix[row][col] < 1 || matrix[row][col] > n) {
                is_valid = false;
                cout << "NO" << endl;
                break;      
            }
            set.insert(matrix[row][col]);
        } 
        cout << endl;
        if (set.size() != (long unsigned int)n)
            is_valid = false;
        
        // cout << "YES" << endl;
    }
//      • Each column contains unique values from 1 - N.
//      • Each of the nXn sub-grids, contains a unique value from 1 - N where, N = n^2


    // Get the execution time in milliseconds
    auto end = HRC::now();
    auto exec_time = chrono::duration_cast<MSEC>(end - start).count();
    cout << exec_time << endl;
    
    return 0;
}

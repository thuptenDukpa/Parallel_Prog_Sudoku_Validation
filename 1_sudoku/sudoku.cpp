#include <iostream>
#include <omp.h>
#include <chrono>
#include <fstream>
#include <vector>
#include <unordered_set>

using namespace std;

typedef chrono::microseconds MSEC;
typedef chrono::high_resolution_clock HRC;

bool check_uniqueness_in_rows(int k, int n, vector<vector<int>>& matrix) {
    for (int row = 0; row < n; row++) {
        unordered_set<int> set;
        //#pragma omp parallel for num_threads(k)
        for (int col = 0; col < n; col++) {
            // Check range of numbers
            if (matrix[row][col] < 1 || matrix[row][col] > n)
                return false;    
        
            set.insert(matrix[row][col]);
        } 
        cout << endl;
        if (set.size() != (long unsigned int)n)
            return false;
    }
    return true;
}

int main() {
    // Start the timer
    auto start = HRC::now();

    // Read the inputs
    ifstream inputFile("../input.txt");
    int k, n;
    inputFile >> k >> n;
    cout << "k (number of threads) = " << k << " n (dimension of sudoku) = " << n << endl;

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
    is_valid = check_uniqueness_in_rows(k, n, matrix);
    
    // Check: Each column contains unique values from 1 - N.

    // Check: Each of the nXn sub-grids, contains a unique value from 1 - N where, N = n^2


    // Get the execution time in milliseconds
    auto end = HRC::now();
    auto exec_time = chrono::duration_cast<MSEC>(end - start).count();
    cout << exec_time << endl;

    // Generate the output
    if (is_valid)
        cout << "Sudoku is valid" << endl;
    else
        cout << "Sudoku is invalid" << endl;
    
    return 0;
}


#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main() {
    ifstream inputFile;
    inputFile.open("../input.txt");
    int k, n;
    if (!inputFile) {
        cout << "Unable to open file" << endl;
        exit(1); // terminate with error
    }
    
    if (inputFile >> k) {
        k = k;
    }
    cout << "k = " << k << " n = " << n;

    vector<vector<int>> matrix(n, vector<int>(n));  
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            inputFile >> matrix[i][j];  
        }
    }
    inputFile.close();
}
To run the files, ensure that you are in the correct directory in your terminal:
cd <project_folder_path>

Input parameters capacity(number of threads), numOps and lambda are present in "inp-params.txt".

The code requires clang to be able to run.

To test the AtomicMRMWRegister, please ensure that, lines 225 and 230 are commented out,
lines 226 and 231 are uncommented and then execute:
g++ -std=c++17 MRMR_ES20BTECH11029.cpp -o MRMR_out -lpthreads
./MRMR_out

To test the inbuilt implementation, please ensure that, lines 226 and 231 are commented out,
lines 225 and 230 are uncommented and then execute:
g++ -std=c++17 MRMR_ES20BTECH11029.cpp -o MRMR_out -lpthreads
./MRMR_out

The output will be written in "LogFile.txt".
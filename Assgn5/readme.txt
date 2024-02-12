To run the files, ensure that you are in the correct directory in your terminal:
cd <project_folder_path>

Input parameters capacity number of threads n, k, lambda1 and lambda2 are present in "inp-params.txt".


To test the CLHLock, execute:
g++ CLH_ES20BTECH11029.cpp -o clh_out 
./clh_out
The output will be written in "CLH-LogFile.txt".

To test the MCSLock, execute:
g++ MCS_ES20BTECH11029.cpp -o mcs_out 
./mcs_out

The output will be written in "MCS-LogFile.txt".
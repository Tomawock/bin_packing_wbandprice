#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Benchmark.cpp"

using namespace std;

int main()
{
    Benchmark instance;
    
    instance.loadWeightBppExtension("../data/N1C1W1_A.BPP");
    cout << instance.summary();
    return 0;
}
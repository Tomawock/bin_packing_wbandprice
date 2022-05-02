#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <vector>
#include <string>

using namespace std;

class Benchmark
{
private:
    int bin_capacity;
    int n_items;
    vector<int> items_weight;

    void extractAttributesFromPath(string path);
public:
    Benchmark();
    void loadWeightBppExtension(string path);
    string summary();
};
#endif
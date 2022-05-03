#include <fstream>
#include <iostream>
#include <string>

#include "Benchmark.h"

Benchmark::Benchmark()
{
}

string Benchmark::summary()
{
    string out = "";
    out += "BIN CAPACITY=" + to_string(this->bin_capacity) + "\n";
    out += "NUM ITEMS=" + to_string(this->n_items) + "\n";
    for (int i = 0; i < this->items_weight.size(); i++)
    {
        out += "W_"+to_string(i)+"_"+ to_string(items_weight[i]) + "\t";
    }
    return out;
}

void Benchmark::extractAttributesFromPath(string path)
{
    // example data N1C1W1_A.BPP read data_format to know what to do
    char x = path[path.size() - 11];
    char y = path[path.size() - 9];

    switch (x)
    {
    case '1':
        this->n_items = 50;
        break;
    case '2':
        this->n_items = 100;
        break;
    case '3':
        this->n_items = 200;
        break;
    case '4':
        this->n_items = 500;
        break;
    default:
        cerr << "Unable to obtain the corret NUMBER OF ITEMS\n";
        cerr << x;
    }

    switch (y)
    {
    case '1':
        this->bin_capacity = 100;
        break;
    case '2':
        this->bin_capacity = 120;
        break;
    case '3':
        this->bin_capacity = 150;
        break;
    default:
        cerr << "Unable to obtain the corret BIN CAPACITY\n";
    }
}

void Benchmark::loadWeightBppExtension(string path)
{
    // set up oher infomation based on the path we are reading
    this->extractAttributesFromPath(path);
    // read the data and load it in the structure

    string line;
    ifstream myfile(path);
    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
            this->items_weight.push_back(stoi(line));
        }
        myfile.close();
        this->items_weight.erase(items_weight.begin(),items_weight.begin()+2);
    }
    else
    {
        cerr << "Unable to open file\n";
    }
};

int Benchmark::getBinCapacity() { return this->bin_capacity; }
int Benchmark::getNumberOfItems() { return this->n_items; }
vector<int> Benchmark::getItemsWeight() { return this->items_weight; }
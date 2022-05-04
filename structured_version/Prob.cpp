#include <vector>
#include <string>
#include <fstream>
#include <iostream> 
#include "gurobi_c++.h"
#include "Item.h"
#include "Prob.h"

Prob::Prob()
{
}
Prob::~Prob()
{
}
void Prob::load(std::string filepath)
{
    char y = filepath[filepath.size() - 9];

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
        std::cerr << "Unable to obtain the corret BIN CAPACITY\n";
    }

    // read the data and load it in the structure
    std::string line;
    std::ifstream myfile(filepath);
    if (myfile.is_open())
    {
        int id = 0;
        while (getline(myfile, line))
        {
            if (id > 1)
            {
                this->items.push_back(new Item(id - 2, stoi(line)));
            }
            id++;
        }
        myfile.close();
    }
    else
    {
        std::cerr << "Unable to open file\n";
    }
}
int Prob::getItemCount()
{
    return this->items.size();
}
Item *Prob::getItem(int id)
{
    return this->items[id];
}
void Prob::show()
{
    //std::cout << "PROB INFORMATION" << std::endl;
    for (int i = 0; i < this->items.size(); i++)
    {
        items[i]->show();
    }
}

std::vector<std::vector<double>> Prob::feasableStart()
{
    std::vector<std::vector<double>> pattern_list;
    int n_items=this->getItemCount();
    for (int i = 0; i < n_items; i++)
    {
        std::vector<double> pattern;
        for (int j = 0; j < n_items; j++)
        {
            if (i == j)
            {
                pattern.push_back(1);
            }
            else
            {
                pattern.push_back(0);
            }
        }
        pattern_list.push_back(pattern);
        pattern.clear();
    }
    return pattern_list;
}

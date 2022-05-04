#include <vector>
#include <string>
#include "gurobi_c++.h"
#include "Item.cpp"

/**
 * This class allows to manipulate the raw data of Problem
 **/
class Prob
{
public:
    std::vector<Item *> items;

    Prob();
    Prob(std::string filepath);
    void load(std::string filepath);
    int getItemCount();
    Item *getItem(int id);
    ~Prob();
};

Prob::Prob()
{
}

Prob::Prob(std::string filepath)
{
}

Prob::~Prob()
{
}

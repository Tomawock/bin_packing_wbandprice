#ifndef PROB_H
#define PROB_H

#include <vector>
#include <string>
#include "Item.h"

/**
 * This class allows to manipulate the raw data of Problem
 **/
class Prob
{
public:
    std::vector<Item*> items;
    int bin_capacity = -1;

    Prob();
    ~Prob();
    void load(std::string filepath);
    int getItemCount();
    Item *getItem(int id);
    void show();
    std::vector<std::vector<double>> feasableStart();
};
#endif
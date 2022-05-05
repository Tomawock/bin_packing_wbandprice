#ifndef COLUMNGENERATION_H
#define COLUMNGENERATION_H

#include "Prob.h"
#include "gurobi_c++.h"

class ColumnGeneration
{
public:
    double time_taken = -1.0;
    double lb=0.0;

    Prob *prob = nullptr;
    GRBEnv *env = nullptr;
    GRBModel *cg_model = nullptr;
    GRBModel *master=nullptr;
    std::vector<std::vector<Item*>> pattern_list;

    ColumnGeneration();
    ColumnGeneration(Prob* prob);
    ~ColumnGeneration();
    void feasableStart();
    void createMaster();
    void createPricing();
    void update();
    void showPatterns();
    //set up the variables 
    void init();
    // do all the algorithm
    double getLB();
    bool price();
};
#endif
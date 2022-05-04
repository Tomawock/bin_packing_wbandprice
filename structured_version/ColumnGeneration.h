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

    ColumnGeneration();
    ~ColumnGeneration();
    void init(GRBModel* master,GRBEnv* env,Prob* prob);
    double getLB();
    void update(GRBModel* master,GRBEnv* env,Prob* prob,std::vector<std::vector<double>> &pattern_list);
    bool price();
};
#endif
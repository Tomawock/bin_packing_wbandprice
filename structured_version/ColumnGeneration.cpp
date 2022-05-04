#include <math.h>
#include "Prob.h"
#include "gurobi_c++.h"
#include "ColumnGeneration.h"

#define EPS 1e-6
#define SEPARATOR "***********************************"

ColumnGeneration::ColumnGeneration()
{
}

ColumnGeneration::~ColumnGeneration()
{
}

void ColumnGeneration::init(GRBModel *master, GRBEnv *env, Prob *prob)
{
    GRBModel relaxed = master->relax();
    std::cout << SEPARATOR << std::endl;
    std::cout << "EXECUTING THE OPTIMIZATION OF THE RELAXED MASTER PB" << std::endl;
    std::cout << SEPARATOR << std::endl;
    relaxed.optimize();
    // get the dual variables values
    GRBConstr *relaxed_constraints = relaxed.getConstrs();
    std::vector<double> dual_variables;
    for (int i = 0; i < relaxed.get(GRB_IntAttr_NumConstrs); i++)
    {
        dual_variables.push_back(relaxed_constraints[i].get(GRB_DoubleAttr_Pi));
    }
    delete relaxed_constraints;
    // defining the pricing model
    cg_model = new GRBModel(*env);
    // varaibles for the pricing problem
    std::vector<GRBVar> y;
    for (int i = 0; i < prob->getItemCount(); i++)
    {
        y.push_back(cg_model->addVar(0.0, 1.0, 0.0, GRB_BINARY, "y_" + std::to_string(i)));
    }
    // forcing the variable to be set inside the model
    cg_model->update();
    // add constraints
    std::vector<GRBConstr> constraints_p;
    GRBLinExpr expr = 0;
    for (int i = 0; i < prob->getItemCount(); i++)
    {
        expr += prob->getItem(i)->w * y[i];
    }
    constraints_p.push_back(cg_model->addConstr(expr <= prob->bin_capacity, "width_constraint"));
    // define the objective function for the pricing problem
    GRBLinExpr obj_function_price = 0;
    for (int i = 0; i < prob->getItemCount(); i++)
    {
        obj_function_price += y[i] * dual_variables[i];
    }
    cg_model->setObjective(obj_function_price, GRB_MAXIMIZE);
    // Force the update of the pricing problem
    cg_model->update();
}

double ColumnGeneration::getLB()
{
    return lb;
}
void ColumnGeneration::update(GRBModel *master, GRBEnv *env, Prob *prob, std::vector<std::vector<double>> &pattern_list)
{
    // Optimize the pricing problem
    cg_model->optimize();
    time_taken = cg_model->get(GRB_DoubleAttr_Runtime);
    lb = cg_model->getObjective().getValue();
    //
    GRBVar *vars = cg_model->getVars();
    // the y are the values of the new pattern
    // now i create the new pattern and add it to the master problem
    std::vector<double> new_pattern;
    for (int i = 0; i < prob->getItemCount(); i++)
    {
        new_pattern.push_back(std::ceil(vars[i].get(GRB_DoubleAttr_X)));
    }
    delete vars;
    pattern_list.push_back(new_pattern);
    new_pattern.clear(); // just to be sure to not do stupid stuff with pointers
    // index of the last pattern added
    int k_index = pattern_list.size() - 1;
    GRBColumn col = GRBColumn();
    for (int i = 0; i < prob->getItemCount(); i++)
    {
        // Add tye constraint only if the variable was selected
        if (pattern_list[k_index][i] > 0)
        {
            col.addTerm(pattern_list[k_index][i], master->getConstr(i));
        }
    }
    // add the new variable related to the last column added, to the list of the master problem variables
    // x.push_back(line after this)
    master->addVar(0.0, 1.0, 1.0, GRB_BINARY, col, "x_p_" + std::to_string(k_index));
    // update the master problem
    master->update();
}
bool ColumnGeneration::price()
{
    if (lb < 1.0 + EPS)
    {
        return true;
    }
    return false;
}

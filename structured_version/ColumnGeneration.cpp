#include <math.h>
#include "Prob.h"
#include "gurobi_c++.h"
#include "ColumnGeneration.h"

#define EPS 1e-6
#define SEPARATOR "***********************************"

ColumnGeneration::ColumnGeneration()
{
}
ColumnGeneration::ColumnGeneration(Prob *prob)
{
    this->prob = prob;
}
ColumnGeneration::~ColumnGeneration()
{
    delete env;
    delete cg_model;
    delete master;
}
void ColumnGeneration::feasableStart(){
    int n_items=prob->getItemCount();
    for (int i = 0; i < n_items; i++)
    {
        std::vector<Item*> pattern;
        for (int j = 0; j < n_items; j++)
        {
            if (i == j)
            {
                pattern.push_back(prob->getItem(i));
            }
        }
        pattern_list.push_back(pattern);
        pattern.clear();
    }
}
void ColumnGeneration::createMaster()
{
    master = new GRBModel(*env);
    // create feasable start to start the lp of the
    feasableStart();
    // add the variables to the master
    std::vector<GRBVar> x;
    for (int k = 0; k < pattern_list.size(); k++)
    {
        x.push_back(master->addVar(0.0, 1.0, 0.0, GRB_BINARY, "x_" + std::to_string(k)));
    }
    std::vector<GRBConstr> constraints;
    for (int i = 0; i < prob->getItemCount(); i++)
    {
        // is not the best way to add expression in c++,
        // read the doc of GRBLinExpr to fix it
        GRBLinExpr expr = 0;
        for (int k = 0; k < pattern_list[i].size(); k++)
        {
            int index_item_selected = pattern_list[i][k]->id;
            expr += 1 * x[index_item_selected];
        }
        constraints.push_back(master->addConstr(expr >= 1, "const_" + std::to_string(i)));
    }
    // define the objective function
    GRBLinExpr obj_function = 0;
    for (int k = 0; k < pattern_list.size(); k++)
    {
        obj_function += x[k];
    }
    master->setObjective(obj_function, GRB_MINIMIZE);
    master->update();
}
void ColumnGeneration::init()
{
    try
    {
        // Create an environment
        env = new GRBEnv(true);
        env->set(GRB_StringParam_LogFile, "../log/master_problem.log");
        env->start();
    }
    catch (GRBException e)
    {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }
    catch (...)
    {
        std::cout << "Exception during optimization" << std::endl;
    }
}
void ColumnGeneration::createPricing()
{
    GRBModel relaxed = master->relax();
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
    try
    {
        // Create the master problem
        createMaster();
        master->write("master_after_creation.lp");
        // Do the column generation
        do
        {
            createPricing();
            update();
        } while (price() && cg_model->getObjective().getValue() > 1.0 + EPS);
        master->optimize();
        //set up actual LB for the master
        lb=master->getObjective().getValue();
    }
    catch (GRBException e)
    {
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }
    catch (...)
    {
        std::cout << "Exception during optimization" << std::endl;
    }

    return lb;
}
void ColumnGeneration::update()
{
    // Optimize the pricing problem
    cg_model->optimize();
    time_taken = cg_model->get(GRB_DoubleAttr_Runtime);
    //
    GRBVar *vars = cg_model->getVars();
    // the y are the values of the new pattern
    // now i create the new pattern and add it to the master problem
    std::vector<Item*> new_pattern;
    for (int i = 0; i < prob->getItemCount(); i++)
    {
        if(std::ceil(vars[i].get(GRB_DoubleAttr_X)) >= 1.0){
            new_pattern.push_back(prob->getItem(i));
        }
    }
    delete vars;
    pattern_list.push_back(new_pattern);
    new_pattern.clear(); // just to be sure to not do stupid stuff with pointers
    // index of the last pattern added
    int k_index = pattern_list.size() - 1;
    GRBColumn col = GRBColumn();    
    for (int i = 0; i <pattern_list[k_index].size(); i++)
    {
        // the value 1.0 could be changed if we have a CSP, but if we have that we will acess the values inise the object
        col.addTerm(1.0, master->getConstr(pattern_list[k_index][i]->id));
    }
    // add the new variable related to the last column added, to the list of the master problem variables
    master->addVar(0.0, 1.0, 1.0, GRB_BINARY, col, "x_p_" + std::to_string(k_index));
    // update the master problem
    master->update();
}
bool ColumnGeneration::price()
{
    int test= cg_model->get(GRB_IntAttr_SolCount);
    if (cg_model->get(GRB_IntAttr_SolCount) > 0)
    {
        return true;
    }
    return false;
}

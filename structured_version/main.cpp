#include "Prob.cpp"
#include "ColumnGeneration.cpp"
#include "Item.cpp"
#include "gurobi_c++.h"

GRBModel* createMaster(std::vector<std::vector<double>> pattern_list,GRBModel* master,Prob prob)
{
    // add the variables to the master
    std::vector<GRBVar> x;
    for (int k = 0; k < pattern_list.size(); k++)
    {
        x.push_back(master->addVar(0.0, 1.0, 0.0, GRB_BINARY, "x_" + std::to_string(k)));
    }
    std::vector<GRBConstr> constraints;
    for (int i = 0; i < prob.getItemCount(); i++)
    {
        // is not the best way to add expression in c++,
        // read the doc of GRBLinExpr to fix it
        GRBLinExpr expr = 0;
        for (int k = 0; k < pattern_list.size(); k++)
        {
            expr += pattern_list[i][k] * x[k];
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
    return master;
}
int main(int argc, char const *argv[])
{
    // Loda the Problem data
    Prob prob;
    prob.load("../../data/N1C1W1_A.BPP");
    prob.show();
    std::cout << prob.getItemCount() << std::endl;

    GRBEnv *env = nullptr;
    GRBModel *master = nullptr;
    try
    {
        // Create an environment
        env = new GRBEnv(true);
        env->set(GRB_StringParam_LogFile, "../log/master_problem.log");
        env->start();

        master = new GRBModel(*env);
        // define the structure for the "patterns" (the result of the pricing problem)
        std::vector<std::vector<double>> pattern_list;
        // create feasable start to start the lp of the
        pattern_list = prob.feasableStart();
        // Create the master problem
        master = createMaster(pattern_list,master,prob);
        master->write("master_after_creation.lp");
        // Do the column generation
        ColumnGeneration cg;
        do 
        {
            cg.init(master,env,&prob);
            cg.update(master,env,&prob,pattern_list);
        }while(!cg.price());
        master->optimize();
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

    // Free local resurces
    delete env;
    delete master;
    return 0;
}

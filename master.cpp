#include "gurobi_c++.h"
#include <string>
#include <math.h>
#include "Benchmark.cpp"

#define MY_EPS 1e-6

using namespace std;

void heuristicFeasableStart(vector<vector<double>> &pattern_list, Benchmark &instance)
{
    vector<double> pattern_values;
    for (int i = 0; i < instance.getNumberOfItems(); i++)
    {
        for (int j = 0; j < instance.getNumberOfItems(); j++)
        {
            if (i == j)
            {
                pattern_values.push_back(1);
            }
            else
            {
                pattern_values.push_back(0);
            }
        }
        pattern_list.push_back(pattern_values);
        pattern_values.clear();
    }
}

int main(int argc, char *argv[])
{
    Benchmark instance;

    instance.loadWeightBppExtension("../data/N1C1W1_A.BPP");
    // cout << instance.summary();
    try
    {
        // Create an environment
        GRBEnv env = GRBEnv(true);

        env.set("LogFile", "../log/master.log");
        env.start();

        // Create an empty model
        GRBModel master = GRBModel(env);

        // Pattern list implement class pattern
        // the first index define the pattern:k-th,
        // the second that has size N define how many time items i-th is selected in one pattern (for bin packing it will be only 1,0 array)
        vector<vector<double>> pattern_list;

        heuristicFeasableStart(pattern_list, instance);

        // add the binary variables for the master problem
        vector<GRBVar> x;
        for (int k = 0; k < pattern_list.size(); k++)
        {
            GRBVar el = master.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x_" + to_string(k));
            x.push_back(el);
        }
        vector<GRBConstr> constraints;
        for (int i = 0; i < instance.getNumberOfItems(); i++)
        {
            // is not the best way to add expression in c++,
            // read the doc of GRBLinExpr to fix it
            GRBLinExpr expr = 0;
            for (int k = 0; k < pattern_list.size(); k++)
            {
                expr += pattern_list[i][k] * x[k];
            }
            constraints.push_back(master.addConstr(expr >= 1, "constraint_for_variable_" + to_string(i)));
        }
        // define the objective function and add it to the master problem
        GRBLinExpr obj_function = 0;
        for (int k = 0; k < pattern_list.size(); k++)
        {
            obj_function += x[k];
        }
        master.setObjective(obj_function, GRB_MINIMIZE);
        // use update to force the model to use the defined constraint and variables
        // GUROBI IS LAZY !!!
        master.update();
        master.write("master.lp");

        int check = 0;
        // do the LP and the column generation
        while (true)
        {
            GRBModel relaxed_model = master.relax();
            relaxed_model.optimize();
            // get the dual variables values
            // pi = [c.Pi for c in relax.getConstrs()]
            GRBConstr *relaxed_constraints = relaxed_model.getConstrs();
            vector<double> dual_variables;
            for (int i = 0; i < relaxed_model.get(GRB_IntAttr_NumConstrs); ++i)
            {
                dual_variables.push_back(relaxed_constraints[i].get(GRB_DoubleAttr_Pi));
            }
            // defining the pricing model
            GRBModel pricing = GRBModel(env);
            // varaibles set-up
            vector<GRBVar> y;
            for (int i = 0; i < instance.getNumberOfItems(); i++)
            {
                GRBVar el = pricing.addVar(0.0, 1.0, 0.0, GRB_BINARY, "y_" + to_string(i));
                y.push_back(el);
            }
            // forcing the variable to be set inside the model
            pricing.update();
            // add constraints
            vector<GRBConstr> constraints_p;
            GRBLinExpr expr = 0;
            for (int i = 0; i < instance.getNumberOfItems(); i++)
            {
                expr += instance.getItemsWeight()[i] * y[i];
            }
            constraints_p.push_back(pricing.addConstr(expr <= instance.getBinCapacity(), "width_constraint"));
            // objective function
            GRBLinExpr obj_function_price = 0;
            for (int i = 0; i < instance.getNumberOfItems(); i++)
            {
                obj_function_price += y[i] * dual_variables[i];
            }
            pricing.setObjective(obj_function_price, GRB_MAXIMIZE);
            pricing.update();
            pricing.optimize();
            //cout << "OBJECTIVE PRICING VALUE\t" << pricing.getObjective().getValue() << endl;
            if (pricing.getObjective().getValue() < 1.0+ MY_EPS)
            {
                cout << "CHECK=" + to_string(check) << endl;
                break;
            }
            GRBVar *vars = pricing.getVars();
            // the y are the values of the new pattern
            // now i create the new pattern and add it to the msater problem
            vector<double> new_pattern;
            for (int i = 0; i < instance.getNumberOfItems(); i++)
            {
                new_pattern.push_back(ceil(vars[i].get(GRB_DoubleAttr_X)));
            }
            // output used just to debug
            // for(int k=0;k<new_pattern.size();k++){
            //     cout << to_string(k)+"_"+to_string(new_pattern[k])+"\t"<<endl;
            // }
            pattern_list.push_back(new_pattern);
            new_pattern.clear(); // just to be sure to not do stupid stuff with pointers
            // intedx of the last pattern added
            int k_index = pattern_list.size() - 1;
            GRBColumn col = GRBColumn();
            for (int i = 0; i < instance.getNumberOfItems(); i++)
            {
                // need to understand why use if
                if (pattern_list[k_index][i] > 0)
                {
                    col.addTerm(pattern_list[k_index][i], constraints[i]);
                }
            }
            // add the new variable related to the last column added
            x[k_index] = master.addVar(0.0, 1.0, 1.0, GRB_BINARY, col, "x_p_" + to_string(k_index));
            master.update();
            // update the check conter to test if all the code works
            check++;
            cout << "CHECK=" + to_string(check) << endl;
        }

        // Do the interg part of the master with the new columns
        master.optimize();
        //master.write("new_master_" + to_string(check) + ".lp");
    }
    catch (GRBException e)
    {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    }
    catch (...)
    {
        cout << "Exception during optimization" << endl;
    }
    return 0;
}
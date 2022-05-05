#include "Prob.cpp"
#include "ColumnGeneration.cpp"
#include "Item.cpp"
#include "gurobi_c++.h"

int main(int argc, char const *argv[])
{
    // Load the Problem data
    Prob prob;
    prob.load("../../data/N1C1W1_A.BPP");
    prob.show();
    std::cout << prob.getItemCount() << std::endl;

    ColumnGeneration cg = ColumnGeneration(&prob);
    cg.init();

    std::cout << "\n\nActual Lower Bound\t" + std::to_string(cg.getLB()) << std::endl;
    return 0;
}

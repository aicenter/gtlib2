//
// Created by Pavel Rytir on 22/01/2018.
//

#include "simpleCplexSolver.h"

#define IL_STD
#include <ilcplex/ilocplex.h>

using std::endl;
using std::cerr;

double solveLP(const int rows, const int cols,
               const vector<double> &utility_matrix,
               vector<double> &solution) {

    // Matrix's rows are indexed by player1. Cols are indexed by player2
    //Utility matrix is for player1. It's zero sum, so utility matrix of player2 is -utility_matrix


    IloEnv env;
    try {
        IloModel model(env);
        IloNumVarArray x(env,cols,0,1,ILOFLOAT);
        IloRangeArray c(env);
        IloNumVar V(env,"V"); // V is utility of player1
        model.add(IloMaximize(env,V));


        // X's are probabilities constraints
        IloExpr probExpr(env);
        for (int i=0; i < cols; ++i) {
            probExpr += x[i];
        }
        IloRange prob(probExpr == 1);
        model.add(prob);

        for (int i=0; i < rows; ++i) {
            IloExpr sum(env);
            for (int j=0; j < cols; ++j) {
                sum += utility_matrix[i*cols+j]*x[j];
            }
            c.add(IloRange(env, 0, sum - V));
        }
        model.add(c);

        IloCplex cplex(model);

        if ( !cplex.solve() ) {
            env.error() << "Failed to optimize LP." << endl;
            throw(-1);
        }

        IloNumArray vals(env);
        env.out() << "Solution status = " << cplex.getStatus() << endl;
        env.out() << "Solution value = " << cplex.getObjValue() << endl;
        cplex.getValues(vals, x);
        //env.out() << "Values = " << vals << endl;

        for (int i=0; i<cols; ++i){
            solution[i] = vals[i];
        }

        return cplex.getObjValue();

    } catch (IloException& e) {
        cerr << "Concert exception caught: " << e << endl;
        return NAN;
    }


}
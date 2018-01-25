//
// Created by Pavel Rytir on 1/20/18.
//

#include "base_old.h"



#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {

    double BestResponse(int player, const shared_ptr<vector<double>> &strategies,
                        int rows, int cols, const vector<double> &utilities) {
        assert(rows * cols == utilities.size());
        double suma;
        if (player == 1) {
            double min = INT32_MAX;
            for (int i = 0; i < rows; i++) {
                suma = 0;
                for (unsigned int j = 0; j < cols; j++) {
                    suma += utilities[i * cols + j] * strategies->operator[](j);
                }
                if (suma < min) min = suma;
            }
            return min;
        }
        double max = INT32_MIN;
        for (int i = 0; i < cols; i++) {
            suma = 0;
            for (unsigned int j = 0; j < rows; j++) {
                suma += utilities[j * cols + i] * strategies->operator[](j);
            }
            if (suma > max) max = suma;
        }
        return max;
    }


    void TreewalkStart(const shared_ptr<Domain> &domain, unsigned int depth) {
        if (depth == 0)
            depth = domain->getMaxDepth();
        vector<Outcome> outcomes = domain->getRootStateDistributionPtr()->GetOutcomes();
        for (Outcome &o : outcomes) {
            Treewalk(domain, o.GetState().get(), depth, domain->getNumberOfPlayers());
        }
    }

    void Treewalk(const shared_ptr<Domain> domain, State *state,
                  unsigned int depth, int players,
                  std::function<void(State *)> FunctionForState) {
        if (state == nullptr) {
            return;
        }

        FunctionForState(state);

        if (depth == 0)
            return;

        auto v = vector<vector<shared_ptr<Action>>>();
        for (int i = 0; i < players; ++i) {
            v.emplace_back(state->getAvailableActionsFor(i));
        }

        auto action = CartProduct(v);
        for (const auto &k : action) {
            ProbDistribution prob = state->PerformAction(k);
            vector<Outcome> outcomes = prob.GetOutcomes();
            for (Outcome &o : outcomes) {
                Treewalk(domain, o.GetState().get(), depth - 1, players, FunctionForState);
            }
        }
    }


    void Treewalk(const shared_ptr<Domain> domain, State *state,
                  unsigned int depth, int players) {
        Treewalk(domain, state, depth, players, [](State *s) {});
    }
}
#pragma clang diagnostic pop
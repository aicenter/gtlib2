//
// Created by Pavel Rytir on 1/20/18.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef PURSUIT_BASE_OLD_H
#define PURSUIT_BASE_OLD_H


#include "base.h"

namespace GTLib2 {

    [[deprecated]]
    double BestResponse(int player, shared_ptr<vector<double>> strategies, int rows,
                        int cols, vector<double> utilities);

// Domain independent treewalk algorithm
    [[deprecated]]
    void Treewalk(shared_ptr<Domain> domain, State *state,
                  unsigned int depth, int players,
                  std::function<void(State *)> FunctionForState);

    [[deprecated]]
    void Treewalk(shared_ptr<Domain> domain, State *state,
                  unsigned int depth, int players);

// Start method for domain independent treewalk algorithm
    [[deprecated]]
    void TreewalkStart(const shared_ptr<Domain> &domain, unsigned int depth = 0);


}

#endif //PURSUIT_BASE_OLD_H

#pragma clang diagnostic pop
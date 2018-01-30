//
// Created by Pavel Rytir on 1/20/18.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef PURSUIT_BASE_OLD_H
#define PURSUIT_BASE_OLD_H


#include "base.h"
#include <unordered_map>

using std::unordered_map;


namespace GTLib2 {






    [[deprecated]]
    double OldBestResponse(int player, shared_ptr<vector<double>> strategies, int rows,
                           int cols, vector<double> utilities);

// Domain independent treewalk algorithm
    [[deprecated]]
    void OldTreewalk(shared_ptr<Domain> domain, State *state,
                     unsigned int depth, int players,
                     std::function<void(State *)> FunctionForState);

    [[deprecated]]
    void OldTreewalk(shared_ptr<Domain> domain, State *state,
                     unsigned int depth, int players);

// Start method for domain independent treewalk algorithm
    [[deprecated]]
    void OldTreewalkStart(const shared_ptr<Domain> &domain, unsigned int depth = 0);


}

#endif //PURSUIT_BASE_OLD_H

#pragma clang diagnostic pop
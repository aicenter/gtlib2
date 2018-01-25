//
// Created by Pavel Rytir on 1/20/18.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef PURSUIT_EFG_OLD_H
#define PURSUIT_EFG_OLD_H

#include "base.h"
#include "efg.h"

namespace GTLib2 {

/**
 * ChanceNode is a class which encapsulates probability distribution,
 * making new EFGNodes from outcomes
 */
    class [[deprecated]] ChanceNode {
    public:
//  // constructor
        ChanceNode(ProbDistribution *prob,
                   const vector<shared_ptr<Action>> &actions,
                   const unique_ptr<EFGNode> &node);

        // GetALL returns a vector of all new EFGNodes.
        vector<unique_ptr<EFGNode>> GetAll();

    private:
        ProbDistribution *prob_;  // probability distribution over the new state
        const vector<shared_ptr<Action>> &actions_;
        const unique_ptr<EFGNode> &node_;  // a current node
    };


    extern unordered_map<size_t, vector<EFGNode>> mapa;


// Domain independent extensive form game treewalk algorithm
    [[deprecated]]
    void EFGTreewalk(const shared_ptr<Domain> &domain, EFGNode *node,
                     unsigned int depth, int players,
                     const vector<shared_ptr<Action>> &list);

    [[deprecated]]
    void EFGTreewalk(const shared_ptr<Domain> &domain, EFGNode *node,
                     unsigned int depth, int players,
                     const vector<shared_ptr<Action>> &list,
                     std::function<void(EFGNode *)> FunctionForState);

//// Start method for domain independent extensive form game treewalk algorithm
    [[deprecated]]
    void EFGTreewalkStart(const shared_ptr<Domain> &domain,
                          unsigned int depth = 0);

    [[deprecated]]
    void EFGTreewalkStart(const shared_ptr<Domain> &domain,
                          std::function<void(EFGNode *)> FunctionForState,
                          unsigned int depth = 0);

}

#endif //PURSUIT_EFG_OLD_H

#pragma clang diagnostic pop
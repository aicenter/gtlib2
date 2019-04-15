/*
    Copyright 2019 Faculty of Electrical Engineering at CTU in Prague

    This file is part of Game Theoretic Library.

    Game Theoretic Library is free software: you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    Game Theoretic Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public 
    License along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/


#include "algorithm.h"

#include <iostream>
#include <chrono>
#include <algorithms/common.h>
#include "base/random.h"

namespace GTLib2 {

using algorithms::createRootEFGNodes;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;


bool playForMicroseconds(unique_ptr<GamePlayingAlgorithm> &alg,
                         const optional<shared_ptr<AOH>> &currentInfoset,
                         long budgetUs) {
    bool continuePlay = true;
    while (budgetUs > 0 && continuePlay) {
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        continuePlay = alg->runPlayIteration(currentInfoset);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(t2 - t1).count();
        budgetUs -= duration;
    }
    if (budgetUs < -100) cerr << "Budget missed by " << budgetUs << " us\n";

    return continuePlay;
}


FixedActionPlayer::FixedActionPlayer(const Domain &domain, Player playingPlayer, int actionIdx)
    : GamePlayingAlgorithm(domain, playingPlayer),
      cache_(InfosetCache(domain_)),
      actionIdx_(actionIdx) {}

bool FixedActionPlayer::runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) {
    if (currentInfoset == nullopt) {
        if (cache_.isCompletelyBuilt()) return true;
        cache_.buildForest();
        return true;
    }

    auto nodes = cache_.getNodesFor(*currentInfoset);
    return !nodes.empty();
}

vector<double> FixedActionPlayer::getPlayDistribution(const shared_ptr<AOH> &currentInfoset) {
    auto nodes = cache_.getNodesFor(currentInfoset);
    // must be signed due to modulo operations
    int numActions = int(nodes[0]->countAvailableActions());
    auto dist = vector<double>(numActions, 0.);
    dist[(numActions + (actionIdx_ % numActions)) % numActions] = 1.;
    return dist;
}

vector<double> playMatch(const Domain &domain,
                         vector<PreparedAlgorithm> algorithmInitializers,
                         vector<int> preplayBudgetMicrosec,
                         vector<int> moveBudgetMicrosec,
                         unsigned long matchSeed) {

    assert(algorithmInitializers.size() == preplayBudgetMicrosec.size() &&
        preplayBudgetMicrosec.size() == moveBudgetMicrosec.size());

    unsigned long numAlgs = algorithmInitializers.size();
    auto algs = vector<unique_ptr<GamePlayingAlgorithm>>(numAlgs);
    auto continuePlay = vector<bool>(numAlgs, true);

    for (int i = 0; i < numAlgs; ++i) {
        algs[i] = algorithmInitializers[i](domain, Player(i));
    }
    for (int i = 0; i < numAlgs; ++i) {
        continuePlay[i] = playForMicroseconds(algs[i], nullopt, preplayBudgetMicrosec[i]);
    }

    auto generator = std::mt19937(matchSeed);
    auto uniformDist = std::uniform_real_distribution<double>(0.0, 1.0);

    shared_ptr<EFGNode> node;
    EFGNodesDistribution nodesDist = createRootEFGNodes(domain.getRootStatesDistribution());
    int chanceAction = pickRandom(nodesDist, uniformDist, generator);
    node = nodesDist[chanceAction].first;

    while (!node->isTerminal()) {
        auto infoset = node->getAOHInfSet();
        auto actions = node->availableActions();
        Player pl = *node->getCurrentPlayer();

        if (continuePlay[pl])
            continuePlay[pl] = playForMicroseconds(algs[pl], infoset, moveBudgetMicrosec[pl]);

        vector<double> probs = continuePlay[pl]
                               ? algs[pl]->getPlayDistribution(infoset)
                               : vector<double>(actions.size(), 1. / actions.size());

        assert(probs.size() == actions.size());
        double sumProbs = 0.0;
        for (double prob : probs) sumProbs += prob;
        assert(fabs(1.0-sumProbs) < 1e-9);

        int playerAction = pickRandom(probs, uniformDist, generator);
        nodesDist = node->performAction(actions[playerAction]);
        chanceAction = pickRandom(nodesDist, uniformDist, generator);
        node = nodesDist[chanceAction].first;
    }

    return node->rewards_;
}

}  // namespace GTLib2

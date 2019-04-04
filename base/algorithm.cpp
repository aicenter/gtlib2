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

using namespace std::chrono;

namespace GTLib2 {

bool playForMicroseconds(unique_ptr<GamePlayingAlgorithm> &alg,
                         const optional<shared_ptr<AOH>> &currentInfoset,
                         long budgetUs) {
    bool continuePlay = true;
    while (budgetUs > 0 && continuePlay) {
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        continuePlay = alg->runIteration(currentInfoset);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(t2 - t1).count();
        budgetUs -= duration;
    }
    if (budgetUs < -100) std::cerr << "Budget missed by " << budgetUs << "us\n";

    return continuePlay;
}


FixedActionPlayer::FixedActionPlayer(const Domain &domain, Player actingPlayer, int action)
    : GamePlayingAlgorithm(domain, actingPlayer),
      _cache(InfosetCache(domain_.getRootStatesDistribution())),
      _action(action) {}

bool FixedActionPlayer::runIteration(const optional<shared_ptr<AOH>> &currentInfoset) {
    if (currentInfoset == nullopt) {
        if (_cache.isCompletelyBuilt()) return true;
        _cache.buildForest();
        return true;
    }

    auto nodes = _cache.getNodesFor(*currentInfoset);
    return !nodes.empty();
}

vector<double> FixedActionPlayer::playDistribution(const shared_ptr<AOH> &currentInfoset) {
    auto nodes = _cache.getNodesFor(currentInfoset);
    int numActions = int(nodes[0]->countAvailableActions()); // must be int due to modulo operations
    auto dist = vector<double>(numActions, 0.);
    dist[(numActions + (_action % numActions)) % numActions] = 1.;
    return dist;
}


int pickAction(const EFGNodesDistribution &probs,
               std::uniform_real_distribution<double> &uniformDist,
               std::mt19937 &generator) {
    double p = uniformDist(generator);
    int i = -1;
    while (p > 0) {
        p -= probs[++i].second;
    }
    assert(i < probs.size());
    return i;
}

int pickAction(const vector<double> &probs,
               std::uniform_real_distribution<double> &uniformDist,
               std::mt19937 &generator) {
    double p = uniformDist(generator);
    int i = -1;
    while (p > 0) {
        p -= probs[++i];
    }
    assert(i < probs.size());
    return i;

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
    EFGNodesDistribution nodesDistribution = algorithms::createRootEFGNodes(
        domain.getRootStatesDistribution());
    int chanceAction = pickAction(nodesDistribution, uniformDist, generator);
    node = nodesDistribution[chanceAction].first;

    while (!node->isTerminal()) {
        auto infoset = node->getAOHInfSet();
        auto actions = node->availableActions();
        Player pl = *node->getCurrentPlayer();

        if (continuePlay[pl])
            continuePlay[pl] = playForMicroseconds(algs[pl], infoset, moveBudgetMicrosec[pl]);

        vector<double> probs = continuePlay[pl]
                               ? algs[pl]->playDistribution(infoset)
                               : vector<double>(actions.size(), 1. / actions.size());

        assert(probs.size() == actions.size());

        int playerAction = pickAction(probs, uniformDist, generator);
        nodesDistribution = node->performAction(actions[playerAction]);
        chanceAction = pickAction(nodesDistribution, uniformDist, generator);
        node = nodesDistribution[chanceAction].first;
    }

    return node->rewards_;
}

}  // namespace GTLib2

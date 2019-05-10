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

#include "base/base.h"
#include "algorithm.h"

#include <chrono>

#include "algorithms/common.h"
#include "base/random.h"

namespace GTLib2 {

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;

bool playForMicroseconds(unique_ptr<GamePlayingAlgorithm> &alg,
                         const optional<shared_ptr<AOH>> &currentInfoset,
                         long budgetUs) {

    PlayControl state = ContinueImproving;
    bool continueImproving = true;
    while (budgetUs > 0 && continueImproving) {
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        state = alg->runPlayIteration(currentInfoset);
        if(state == StopImproving || state == GiveUp) continueImproving = false;
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(t2 - t1).count();
        budgetUs -= duration;
    }
    if (budgetUs < -100) cerr << "Budget missed by " << budgetUs << " us\n";

    return state != GiveUp;
}

FixedActionPlayer::FixedActionPlayer(const Domain &domain, Player playingPlayer, int actionIdx)
    : GamePlayingAlgorithm(domain, playingPlayer),
      cache_(InfosetCache(domain_)),
      actionIdx_(actionIdx) {}

PlayControl FixedActionPlayer::runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) {
    if (cache_.isCompletelyBuilt()) return StopImproving;
    cache_.buildForest();
    return StopImproving;
}

optional<ProbDistribution>
FixedActionPlayer::getPlayDistribution(const shared_ptr<AOH> &currentInfoset) {
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

    shared_ptr<EFGNode> node = createRootEFGNode(domain.getRootStatesDistribution());

    while (node->type_ != TerminalNode) {
        int playerAction;
        auto actions = node->availableActions();

        switch(node->type_) {
            case ChanceNode:
                playerAction = pickRandom(*node, generator);
                break;

            case PlayerNode:
            {
                auto infoset = node->getAOHInfSet();
                Player pl = node->getPlayer();

                if (continuePlay[pl])
                    continuePlay[pl] = playForMicroseconds(algs[pl], infoset, moveBudgetMicrosec[pl]);

                ProbDistribution probs;
                if(continuePlay[pl]) {
                    auto maybeProbs = algs[pl]->getPlayDistribution(infoset);
                    if (maybeProbs == nullopt) continuePlay[pl] = false;
                    else probs = *maybeProbs;
                }
                if(!continuePlay[pl]) {
                    probs = ProbDistribution(actions.size(), 1. / actions.size());
                }

                assert(probs.size() == actions.size());
                double sumProbs = 0.0;
                for (double prob : probs) sumProbs += prob;
                assert(fabs(1.0 - sumProbs) < 1e-9);

                playerAction = pickRandom(probs, generator);
                break;
            }

            case TerminalNode:
                assert(false); // unreachable
                break;
            default:
                assert(false); // unrecognized option!
        }

        node = node->performAction(actions[playerAction]);
    }

    return node->getUtilities();
}

}  // namespace GTLib2

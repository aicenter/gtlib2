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

#include "base/algorithm.h"

#include <chrono>

#include "base/random.h"
#include "algorithms/common.h"

namespace GTLib2 {

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;

bool playForBudget(GamePlayingAlgorithm &alg,
                   const optional<shared_ptr<AOH>> &currentInfoset,
                   unsigned int budgetValue, BudgetType type) {

    switch (type) {
        case BudgetTime:
            return playForMilliseconds(alg, currentInfoset, budgetValue);
        case BudgetIterations:
            return playForIterations(alg, currentInfoset, budgetValue);
        default:
            assert(false); // unrecognized type!
    }
}

bool playForMilliseconds(GamePlayingAlgorithm &alg,
                         const optional<shared_ptr<AOH>> &currentInfoset,
                         unsigned int budgetMs) {

    PlayControl state = ContinueImproving;
    bool continueImproving = true;
    long budgetUs = budgetMs * 1000;
    while (budgetUs > 0 && continueImproving) {
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        state = alg.runPlayIteration(currentInfoset);
        if (state == StopImproving || state == GiveUp) continueImproving = false;
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(t2 - t1).count();
        budgetUs -= duration;
    }
    if (budgetUs < -1000) LOG_WARN("Budget missed by " << -1 * budgetUs / 1000. << " ms")

    return state != GiveUp;
}

bool playForIterations(GamePlayingAlgorithm &alg,
                       const optional<shared_ptr<AOH>> &currentInfoset,
                       unsigned int budgetIters) {

    PlayControl state = ContinueImproving;
    bool continueImproving = true;
    while (budgetIters > 0 && continueImproving) {
        state = alg.runPlayIteration(currentInfoset);
        if (state == StopImproving || state == GiveUp) continueImproving = false;
        budgetIters--;
    }
    return state != GiveUp;
}

FixedActionPlayer::FixedActionPlayer(const Domain &domain, Player playingPlayer, int actionIdx)
    : GamePlayingAlgorithm(domain, playingPlayer),
      cache_(InfosetCache(domain_)),
      actionIdx_(actionIdx) {}

PlayControl FixedActionPlayer::runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) {
    if (cache_.isCompletelyBuilt()) return StopImproving;
    cache_.buildTree();
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
                         vector<unsigned int> preplayBudget,
                         vector<unsigned int> moveBudget,
                         BudgetType simulationType,
                         unsigned long matchSeed) {

#define LOG_PLAYER(pl, msg) LOG_INFO(CLI::set_color(CLI::Color(pl+6)) << msg << CLI::set_color())

    assert(algorithmInitializers.size() == preplayBudget.size() &&
        preplayBudget.size() == moveBudget.size());

    unsigned long numAlgs = algorithmInitializers.size();
    auto algs = vector<unique_ptr<GamePlayingAlgorithm>>(numAlgs);
    auto continuePlay = vector<bool>(numAlgs, true);

    for (int i = 0; i < numAlgs; ++i) {
        LOG_PLAYER(i, "Initializing player " << i)
        algs[i] = algorithmInitializers[i](domain, Player(i));
    }
    for (int i = 0; i < numAlgs; ++i) {
        LOG_PLAYER(i, "Player " << i << " is thinking in preplay")
        continuePlay[i] = playForBudget(*algs[i], PLAY_FROM_ROOT, preplayBudget[i], simulationType);
    }

    auto generator = std::mt19937(matchSeed);

    shared_ptr<EFGNode> node = createRootEFGNode(domain.getRootStatesDistribution());
//    node->
    while (node->type_ != TerminalNode) {
        int playerAction;
        auto actions = node->availableActions();

        switch (node->type_) {
            case ChanceNode:
                playerAction = pickRandom(*node, generator);
                LOG_PLAYER(2, "Chance picked action " << playerAction)
                break;

            case PlayerNode: {
                auto infoset = node->getAOHInfSet();
                Player pl = node->getPlayer();

                if (continuePlay[pl]) {
                    LOG_PLAYER(pl, "Player " << int(pl) << " is thinking...")
                    continuePlay[pl] = playForBudget(*algs[pl], infoset,
                                                     moveBudget[pl], simulationType);
                }

                ProbDistribution probs;
                if (continuePlay[pl]) {
                    auto maybeProbs = algs[pl]->getPlayDistribution(infoset);
                    if (maybeProbs == nullopt) continuePlay[pl] = false;
                    else probs = *maybeProbs;
                }
                if (!continuePlay[pl]) {
                    LOG_PLAYER(pl, "Player " << int(pl) << " gave up!")
                    probs = ProbDistribution(actions.size(), 1. / actions.size());
                }

                assert(probs.size() == actions.size());
                double sumProbs = 0.0;
                for (double prob : probs) sumProbs += prob;
                assert(fabs(1.0 - sumProbs) < 1e-9);

                playerAction = pickRandom(probs, generator);
                LOG_PLAYER(pl, "Player " << int(pl) << " picked action " << playerAction
                                         << " from distr " << probs)
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
